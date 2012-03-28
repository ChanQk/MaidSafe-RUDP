/*******************************************************************************
 *  Copyright 2012 MaidSafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of MaidSafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the licence   *
 *  file licence.txt found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of MaidSafe.net. *
 ******************************************************************************/
// Original author: Christopher M. Kohlhoff (chris at kohlhoff dot com)

#ifndef MAIDSAFE_TRANSPORT_RUDP_KEEPALIVE_PACKET_H_
#define MAIDSAFE_TRANSPORT_RUDP_KEEPALIVE_PACKET_H_

#include "boost/asio/buffer.hpp"
#include "maidsafe/transport/rudp_control_packet.h"

namespace maidsafe {

namespace transport {

class RudpKeepalivePacket : public RudpControlPacket {
 public:
  enum { kPacketSize = RudpControlPacket::kHeaderSize };
  enum { kPacketType = 1 };

  RudpKeepalivePacket();
  virtual ~RudpKeepalivePacket() {}

  static bool IsValid(const boost::asio::const_buffer &buffer);
  bool Decode(const boost::asio::const_buffer &buffer);
  size_t Encode(const boost::asio::mutable_buffer &buffer) const;
};

}  // namespace transport

}  // namespace maidsafe

#endif  // MAIDSAFE_TRANSPORT_RUDP_KEEPALIVE_PACKET_H_
