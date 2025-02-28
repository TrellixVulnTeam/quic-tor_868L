// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "quic_simple_client.h"

#include "base/logging.h"
#include "base/thread_task_runner_handle.h"
#include "base/single_thread_task_runner.h"
#include "net/base/net_errors.h"
#include "net/quic/crypto/quic_random.h"
#include "net/quic/quic_connection.h"
#include "net/quic/quic_flags.h"
#include "net/quic/quic_protocol.h"
#include "net/quic/quic_server_id.h"

#include "custom/run_loop.h"
#include "custom/udp/udp_client_socket.h"
#include "custom/quic_simple_connection_helper.h"
#include "custom/quic_simple_packet_reader.h"
#include "custom/quic_default_packet_writer.h"

using std::string;
using std::vector;

namespace net {
namespace tools {

void QuicSimpleClient::ClientQuicDataToResend::Resend() {
  client_->SendRequest(body_, fin_);
}

QuicSimpleClient::QuicSimpleClient(IPEndPoint server_address,
                                   const QuicServerId& server_id,
                                   const QuicVersionVector& supported_versions,
                                   ProofVerifier* proof_verifier)
    : QuicClientBase(server_id,
                     supported_versions,
                     QuicConfig(),
                     CreateQuicConnectionHelper(),
                     proof_verifier),
      server_address_(server_address),
      local_port_(0),
      initialized_(false),
      packet_reader_started_(false),
      weak_factory_(this) {}

QuicSimpleClient::QuicSimpleClient(IPEndPoint server_address,
                                   const QuicServerId& server_id,
                                   const QuicVersionVector& supported_versions,
                                   const QuicConfig& config,
                                   ProofVerifier* proof_verifier)
    : QuicClientBase(server_id,
                     supported_versions,
                     config,
                     CreateQuicConnectionHelper(),
                     proof_verifier),
      server_address_(server_address),
      local_port_(0),
      initialized_(false),
      packet_reader_started_(false),
      weak_factory_(this) {}

QuicSimpleClient::~QuicSimpleClient() {
  if (connected()) {
    session()->connection()->SendConnectionClosePacket(QUIC_PEER_GOING_AWAY,
                                                       "");
  }
  STLDeleteElements(&data_to_resend_on_connect_);
  STLDeleteElements(&data_sent_before_handshake_);
}

bool QuicSimpleClient::Initialize() {
  DCHECK(!initialized_);

  QuicClientBase::Initialize();

  if (!CreateUDPSocket()) {
    return false;
  }

  initialized_ = true;
  return true;
}

QuicSimpleClient::QuicDataToResend::QuicDataToResend(StringPiece body,
                                                     bool fin)
    : body_(body), fin_(fin) {}

QuicSimpleClient::QuicDataToResend::~QuicDataToResend() {
}

bool QuicSimpleClient::CreateUDPSocket() {
  scoped_ptr<UDPClientSocket> socket(
      new UDPClientSocket(DatagramSocket::DEFAULT_BIND, RandIntCallback(),
                          &net_log_, NetLog::Source()));

  int address_family = server_address_.GetSockAddrFamily();
  if (bind_to_address_.size() != 0) {
    client_address_ = IPEndPoint(bind_to_address_, local_port_);
  } else if (address_family == AF_INET) {
    IPAddressNumber any4;
    CHECK(net::ParseIPLiteralToNumber("0.0.0.0", &any4));
    client_address_ = IPEndPoint(any4, local_port_);
  } else {
    IPAddressNumber any6;
    CHECK(net::ParseIPLiteralToNumber("::", &any6));
    client_address_ = IPEndPoint(any6, local_port_);
  }

  int rc = socket->Connect(server_address_);
  if (rc != OK) {
    LOG(ERROR) << "Connect failed: " << ErrorToShortString(rc);
    return false;
  }

  rc = socket->SetReceiveBufferSize(kDefaultSocketReceiveBuffer);
  if (rc != OK) {
    LOG(ERROR) << "SetReceiveBufferSize() failed: " << ErrorToShortString(rc);
    return false;
  }

  rc = socket->SetSendBufferSize(kDefaultSocketReceiveBuffer);
  if (rc != OK) {
    LOG(ERROR) << "SetSendBufferSize() failed: " << ErrorToShortString(rc);
    return false;
  }

  rc = socket->GetLocalAddress(&client_address_);
  if (rc != OK) {
    LOG(ERROR) << "GetLocalAddress failed: " << ErrorToShortString(rc);
    return false;
  }

  socket_.swap(socket);
  packet_reader_.reset(new QuicSimplePacketReader(
      socket_.get(), &clock_, this, kQuicYieldAfterPacketsRead,
      QuicTime::Delta::FromMilliseconds(kQuicYieldAfterDurationMilliseconds)));

  if (socket != nullptr) {
    socket->Close();
  }

  return true;
}

void QuicSimpleClient::StartPacketReaderIfNotStarted() {
  if (!packet_reader_started_) {
    packet_reader_->StartReading();
    packet_reader_started_ = true;
  }
}

bool QuicSimpleClient::Connect() {
  // Attempt multiple connects until the maximum number of client hellos have
  // been sent.
  while (!connected() &&
         GetNumSentClientHellos() <= QuicCryptoClientSimpleStream::kMaxClientHellos) {
    StartConnect();
    StartPacketReaderIfNotStarted();
    while (EncryptionBeingEstablished()) {
      WaitForEvents();
    }
    if (FLAGS_enable_quic_stateless_reject_support && connected() &&
        !data_to_resend_on_connect_.empty()) {
      // A connection has been established and there was previously queued data
      // to resend.  Resend it and empty the queue.
      for (QuicDataToResend* data : data_to_resend_on_connect_) {
        data->Resend();
      }
      STLDeleteElements(&data_to_resend_on_connect_);
    }
    if (session() != nullptr &&
        session()->error() != QUIC_CRYPTO_HANDSHAKE_STATELESS_REJECT) {
      // We've successfully created a session but we're not connected, and there
      // is no stateless reject to recover from.  Give up trying.
      break;
    }
  }
  if (!connected() &&
      GetNumSentClientHellos() > QuicCryptoClientSimpleStream::kMaxClientHellos &&
      session() != nullptr &&
      session()->error() == QUIC_CRYPTO_HANDSHAKE_STATELESS_REJECT) {
    // The overall connection failed due too many stateless rejects.
    set_connection_error(QUIC_CRYPTO_TOO_MANY_REJECTS);
  }
  return session()->connection()->connected();
}

void QuicSimpleClient::StartConnect() {
  DCHECK(initialized_);
  DCHECK(!connected());

  set_writer(CreateQuicPacketWriter());

  DummyPacketWriterFactory factory(writer());

  if (connected_or_attempting_connect()) {
    // Before we destroy the last session and create a new one, gather its stats
    // and update the stats for the overall connection.
    UpdateStats();
    if (session()->error() == QUIC_CRYPTO_HANDSHAKE_STATELESS_REJECT) {
      // If the last error was due to a stateless reject, queue up the data to
      // be resent on the next successful connection.
      // TODO(jokulik): I'm a little bit concerned about ordering here.  Maybe
      // we should just maintain one queue?
      DCHECK(data_to_resend_on_connect_.empty());
      data_to_resend_on_connect_.swap(data_sent_before_handshake_);
    }
  }

  CreateQuicClientSimpleSession(new QuicConnection(
      GetNextConnectionId(), server_address_, helper(), factory,
      /* owns_writer= */ false, Perspective::IS_CLIENT, supported_versions()));

  session()->Initialize();
  session()->CryptoConnect();
  set_connected_or_attempting_connect(true);
}

void QuicSimpleClient::Disconnect() {
  DCHECK(initialized_);

  if (connected()) {
    session()->connection()->SendConnectionClose(QUIC_PEER_GOING_AWAY);
  }
  STLDeleteElements(&data_to_resend_on_connect_);
  STLDeleteElements(&data_sent_before_handshake_);

  reset_writer();
  packet_reader_.reset();

  initialized_ = false;
}

void QuicSimpleClient::SendRequest(StringPiece body,
                                   bool fin) {
  QuicClientSimpleStream* stream = CreateReliableClientStream();
  if (stream == nullptr) {
    LOG(DFATAL) << "stream creation failed!";
    return;
  }
  stream->set_visitor(this);
  stream->SendRequest(body, fin);
  if (FLAGS_enable_quic_stateless_reject_support) {
    // Record this in case we need to resend.
    auto data_to_resend =
        new ClientQuicDataToResend(body, fin, this);
    MaybeAddQuicDataToResend(data_to_resend);
  }
}

void QuicSimpleClient::MaybeAddQuicDataToResend(
    QuicDataToResend* data_to_resend) {
  DCHECK(FLAGS_enable_quic_stateless_reject_support);
  if (session()->IsCryptoHandshakeConfirmed()) {
    // The handshake is confirmed.  No need to continue saving requests to
    // resend.
    STLDeleteElements(&data_sent_before_handshake_);
    delete data_to_resend;
    return;
  }

  // The handshake is not confirmed.  Push the data onto the queue of data to
  // resend if statelessly rejected.
  data_sent_before_handshake_.push_back(data_to_resend);
}

void QuicSimpleClient::SendRequestAndWaitForResponse(
    StringPiece body,
    bool fin) {
  SendRequest(body, fin);
  while (WaitForEvents()) {
  }
}

bool QuicSimpleClient::WaitForEvents() {
  DCHECK(connected());

  base::RunLoop().RunUntilIdle();

  DCHECK(session() != nullptr);
  if (!connected() &&
      session()->error() == QUIC_CRYPTO_HANDSHAKE_STATELESS_REJECT) {
    DCHECK(FLAGS_enable_quic_stateless_reject_support);
    DVLOG(1) << "Detected stateless reject while waiting for events.  "
             << "Attempting to reconnect.";
    Connect();
  }

  return session()->num_active_requests() != 0;
}

bool QuicSimpleClient::MigrateSocket(const IPAddressNumber& new_host) {
  if (!connected()) {
    return false;
  }

  bind_to_address_ = new_host;
  if (!CreateUDPSocket()) {
    return false;
  }

  session()->connection()->SetSelfAddress(client_address_);

  QuicPacketWriter* writer = CreateQuicPacketWriter();
  DummyPacketWriterFactory factory(writer);
  set_writer(writer);
  session()->connection()->SetQuicPacketWriter(writer, false);

  return true;
}

void QuicSimpleClient::OnClose(QuicSimpleStream* stream) {
  DCHECK(stream != nullptr);
  QuicClientSimpleStream* client_stream =
      static_cast<QuicClientSimpleStream*>(stream);
  /*
  SpdyHeadersToHttpResponse(client_stream->headers(), net::HTTP2, &response);
  if (response_listener_.get() != nullptr) {
    response_listener_->OnCompleteResponse(stream->id(), *response.headers,
                                           client_stream->data());
  }
  */

  // Store response headers and body.
  if (store_response_) {
    latest_response_body_ = client_stream->data();
  }
}

const string& QuicSimpleClient::latest_response_body() const {
  LOG_IF(DFATAL, !store_response_) << "Response not stored!";
  return latest_response_body_;
}

QuicConnectionId QuicSimpleClient::GenerateNewConnectionId() {
  return helper()->GetRandomGenerator()->RandUint64();
}

QuicSimpleConnectionHelper* QuicSimpleClient::CreateQuicConnectionHelper() {
  return new QuicSimpleConnectionHelper(base::ThreadTaskRunnerHandle::Get().get(),
                                  &clock_, QuicRandom::GetInstance());
}

QuicPacketWriter* QuicSimpleClient::CreateQuicPacketWriter() {
  return new QuicDefaultPacketWriter(socket_.get());
}

void QuicSimpleClient::OnReadError(int result,
                                   const DatagramClientSocket* socket) {
  LOG(ERROR) << "QuicSimpleClient read failed: " << ErrorToShortString(result);
  Disconnect();
}

bool QuicSimpleClient::OnPacket(const QuicEncryptedPacket& packet,
                                IPEndPoint local_address,
                                IPEndPoint peer_address) {
  session()->connection()->ProcessUdpPacket(local_address, peer_address,
                                            packet);
  if (!session()->connection()->connected()) {
    return false;
  }

  return true;
}

}  // namespace tools
}  // namespace net
