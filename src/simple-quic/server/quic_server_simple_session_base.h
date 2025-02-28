// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// A server specific QuicSession subclass.

#ifndef NET_TOOLS_QUIC_QUIC_SERVER_SIMPLE_SESSION_BASE_H_
#define NET_TOOLS_QUIC_QUIC_SERVER_SIMPLE_SESSION_BASE_H_

#include <stdint.h>

#include <set>
#include <string>
#include <vector>

#include "base/containers/hash_tables.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "net/quic/quic_crypto_server_stream.h"
#include "net/quic/quic_protocol.h"

#include "custom/quic_simple_session.h"
#include "server/quic_server_simple_session_visitor.h"

namespace net {

class QuicBlockedWriterInterface;
class QuicConfig;
class QuicConnection;
class QuicCryptoServerConfig;
class ReliableQuicStream;

namespace tools {

class QuicServerSimpleSessionBase : public QuicSimpleSession {
 public:
  // |crypto_config| must outlive the session.
  QuicServerSimpleSessionBase(const QuicConfig& config,
                        QuicConnection* connection,
                        QuicServerSimpleSessionVisitor* visitor,
                        const QuicCryptoServerConfig* crypto_config);

  // Override the base class to notify the owner of the connection close.
  void OnConnectionClosed(QuicErrorCode error, bool from_peer) override;
  void OnWriteBlocked() override;

  // Sends a server config update to the client, containing new bandwidth
  // estimate.
  void OnCongestionWindowChange(QuicTime now) override;

  ~QuicServerSimpleSessionBase() override;

  void Initialize() override;

  const QuicCryptoServerStreamBase* crypto_stream() const {
    return crypto_stream_.get();
  }

  // Override base class to process FEC config received from client.
  void OnConfigNegotiated() override;

  void set_serving_region(const std::string& serving_region) {
    serving_region_ = serving_region;
  }

 protected:
  // QuicSession methods(override them with return type of QuicSpdyStream*):
  QuicCryptoServerStreamBase* GetCryptoStream() override;

  // If an outgoing stream can be created, return true.
  // Return false when connection is closed or forward secure encryption hasn't
  // established yet or number of server initiated streams already reaches the
  // upper limit.
  bool ShouldCreateOutgoingDynamicStream();

  // If we should create an incoming stream, returns true. Otherwise
  // does error handling, including communicating the error to the client and
  // possibly closing the connection, and returns false.
  virtual bool ShouldCreateIncomingDynamicStream(QuicStreamId id);

  virtual QuicCryptoServerStreamBase* CreateQuicCryptoServerStream(
      const QuicCryptoServerConfig* crypto_config) = 0;

  const QuicCryptoServerConfig* crypto_config() { return crypto_config_; }

 private:
  const QuicCryptoServerConfig* crypto_config_;
  scoped_ptr<QuicCryptoServerStreamBase> crypto_stream_;
  QuicServerSimpleSessionVisitor* visitor_;

  // Whether bandwidth resumption is enabled for this connection.
  bool bandwidth_resumption_enabled_;

  // The most recent bandwidth estimate sent to the client.
  QuicBandwidth bandwidth_estimate_sent_to_client_;

  // Text describing server location. Sent to the client as part of the bandwith
  // estimate in the source-address token. Optional, can be left empty.
  std::string serving_region_;

  // Time at which we send the last SCUP to the client.
  QuicTime last_scup_time_;

  // Number of packets sent to the peer, at the time we last sent a SCUP.
  int64_t last_scup_packet_number_;

  DISALLOW_COPY_AND_ASSIGN(QuicServerSimpleSessionBase);
};

}  // namespace tools
}  // namespace net

#endif  // NET_TOOLS_QUIC_QUIC_SERVER_SIMPLE_SESSION_BASE_H_
