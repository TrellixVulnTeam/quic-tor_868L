// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "quic_client_base.h"

#include "net/quic/crypto/quic_random.h"
#include "net/quic/quic_server_id.h"

namespace net {
namespace tools {

QuicClientBase::QuicClientBase(const QuicServerId& server_id,
                               const QuicVersionVector& supported_versions,
                               const QuicConfig& config,
                               QuicConnectionHelperInterface* helper,
                               ProofVerifier* proof_verifier)
    : server_id_(server_id),
      config_(config),
      crypto_config_(proof_verifier),
      helper_(helper),
      supported_versions_(supported_versions),
      initial_max_packet_length_(0),
      num_stateless_rejects_received_(0),
      num_sent_client_hellos_(0),
      connection_error_(QUIC_NO_ERROR),
      connected_or_attempting_connect_(false) {}

QuicClientBase::~QuicClientBase() {}

bool QuicClientBase::Initialize() {
  num_sent_client_hellos_ = 0;
  num_stateless_rejects_received_ = 0;
  connection_error_ = QUIC_NO_ERROR;
  connected_or_attempting_connect_ = false;
  return true;
}

QuicClientBase::DummyPacketWriterFactory::DummyPacketWriterFactory(
    QuicPacketWriter* writer)
    : writer_(writer) {}

QuicClientBase::DummyPacketWriterFactory::~DummyPacketWriterFactory() {}

QuicPacketWriter* QuicClientBase::DummyPacketWriterFactory::Create(
    QuicConnection* /*connection*/) const {
  return writer_;
}

ProofVerifier* QuicClientBase::proof_verifier() const {
  return crypto_config_.proof_verifier();
}

QuicClientSimpleSession* QuicClientBase::CreateQuicClientSimpleSession(
    QuicConnection* connection) {
  session_.reset(
      new QuicClientSimpleSession(config_, connection, server_id_, &crypto_config_));
  if (initial_max_packet_length_ != 0) {
    session()->connection()->SetMaxPacketLength(initial_max_packet_length_);
  }
  return session_.get();
}

bool QuicClientBase::EncryptionBeingEstablished() {
  return !session_->IsEncryptionEstablished() &&
         session_->connection()->connected();
}

QuicClientSimpleStream* QuicClientBase::CreateReliableClientStream() {
  if (!connected()) {
    return nullptr;
  }

  return session_->CreateOutgoingDynamicStream(kDefaultPriority);
}

void QuicClientBase::WaitForStreamToClose(QuicStreamId id) {
  DCHECK(connected());

  while (connected() && !session_->IsClosedStream(id)) {
    WaitForEvents();
  }
}

void QuicClientBase::WaitForCryptoHandshakeConfirmed() {
  DCHECK(connected());

  while (connected() && !session_->IsCryptoHandshakeConfirmed()) {
    WaitForEvents();
  }
}

bool QuicClientBase::connected() const {
  return session_.get() && session_->connection() &&
         session_->connection()->connected();
}

bool QuicClientBase::goaway_received() const {
  return session_ != nullptr && session_->goaway_received();
}

int QuicClientBase::GetNumSentClientHellos() {
  // If we are not actively attempting to connect, the session object
  // corresponds to the previous connection and should not be used.
  const int current_session_hellos = !connected_or_attempting_connect_
                                         ? 0
                                         : session_->GetNumSentClientHellos();
  return num_sent_client_hellos_ + current_session_hellos;
}

void QuicClientBase::UpdateStats() {
  num_sent_client_hellos_ += session()->GetNumSentClientHellos();
  if (session()->error() == QUIC_CRYPTO_HANDSHAKE_STATELESS_REJECT) {
    ++num_stateless_rejects_received_;
  }
}

QuicErrorCode QuicClientBase::connection_error() const {
  // Return the high-level error if there was one.  Otherwise, return the
  // connection error from the last session.
  if (connection_error_ != QUIC_NO_ERROR) {
    return connection_error_;
  }
  if (session_.get() == nullptr) {
    return QUIC_NO_ERROR;
  }
  return session_->error();
}

QuicConnectionId QuicClientBase::GetNextConnectionId() {
  QuicConnectionId server_designated_id = GetNextServerDesignatedConnectionId();
  return server_designated_id ? server_designated_id
                              : GenerateNewConnectionId();
}

QuicConnectionId QuicClientBase::GetNextServerDesignatedConnectionId() {
  QuicCryptoClientConfig::CachedState* cached =
      crypto_config_.LookupOrCreate(server_id_);
  // If the cached state indicates that we should use a server-designated
  // connection ID, then return that connection ID.
  CHECK(cached != nullptr) << "QuicClientCryptoConfig::LookupOrCreate returned "
                           << "unexpected nullptr.";
  return cached->has_server_designated_connection_id()
             ? cached->GetNextServerDesignatedConnectionId()
             : 0;
}

QuicConnectionId QuicClientBase::GenerateNewConnectionId() {
  return QuicRandom::GetInstance()->RandUint64();
}

}  // namespace tools
}  // namespace net
