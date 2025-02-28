// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef NET_QUIC_QUIC_SIMPLE_PACKET_READER_H_
#define NET_QUIC_QUIC_SIMPLE_PACKET_READER_H_

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "net/base/io_buffer.h"
#include "net/base/net_export.h"
#include "net/quic/quic_protocol.h"

#include "custom/udp/datagram_client_socket.h"

namespace net {

class QuicClock;
class QuicTime;

// If more than this many packets have been read or more than that many
// milliseconds have passed, QuicSimplePacketReader::StartReading() yields by doing a
// QuicSimplePacketReader::PostTask().
const int kQuicYieldAfterPacketsRead = 32;
const int kQuicYieldAfterDurationMilliseconds = 20;

class NET_EXPORT_PRIVATE QuicSimplePacketReader {
 public:
  class NET_EXPORT_PRIVATE Visitor {
   public:
    virtual ~Visitor(){};
    virtual void OnReadError(int result,
                             const DatagramClientSocket* socket) = 0;
    virtual bool OnPacket(const QuicEncryptedPacket& packet,
                          IPEndPoint local_address,
                          IPEndPoint peer_address) = 0;
  };

  QuicSimplePacketReader(DatagramClientSocket* socket,
                   QuicClock* clock,
                   Visitor* visitor,
                   int yield_after_packets,
                   QuicTime::Delta yield_after_duration);
  virtual ~QuicSimplePacketReader();

  // Causes the QuicConnectionHelper to start reading from the socket
  // and passing the data along to the QuicConnection.
  void StartReading();

 private:
  // A completion callback invoked when a read completes.
  void OnReadComplete(int result);

  DatagramClientSocket* socket_;
  Visitor* visitor_;
  bool read_pending_;
  int num_packets_read_;
  QuicClock* clock_;  // Owned by QuicStreamFactory
  int yield_after_packets_;
  QuicTime::Delta yield_after_duration_;
  QuicTime yield_after_;
  scoped_refptr<IOBufferWithSize> read_buffer_;

  base::WeakPtrFactory<QuicSimplePacketReader> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(QuicSimplePacketReader);
};

}  // namespace net

#endif  // NET_QUIC_QUIC_SIMPLE_PACKET_READER_H_
