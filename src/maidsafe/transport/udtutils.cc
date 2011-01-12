﻿
/* Copyright (c) 2010 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "maidsafe/transport/udtutils.h"
#include <boost/lexical_cast.hpp>
#include <string>
#include "maidsafe/common/log.h"

namespace maidsafe {

namespace transport {

namespace udtutils {

boost::shared_ptr<addrinfo const> SocketGetAddrinfo(char const *node,
                                                    char const *service,
                                                    addrinfo const &hints,
                                                    int *result) {
  addrinfo *servinfo;
  *result = getaddrinfo(node, service, &hints, &servinfo);
  return (*result == 0) ? boost::shared_ptr<addrinfo>(servinfo, freeaddrinfo) :
                          boost::shared_ptr<addrinfo>();
}

boost::shared_ptr<addrinfo const> Next(
    boost::shared_ptr<addrinfo const> const &node) {
  return boost::shared_ptr<addrinfo const>(node, node->ai_next);
}

TransportCondition GetNewSocket(
    const Endpoint &endpoint,
    bool reuse_address,
    transport::SocketId *socket_id,
    boost::shared_ptr<addrinfo const> *address_info) {
  if (socket_id == NULL)
    return kConnectError;

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  bool empty_ip(endpoint.ip == IP());
  hints.ai_flags = (empty_ip ? AI_PASSIVE : 0);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  const char *address(empty_ip ? NULL : endpoint.ip.to_string().c_str());
  std::string port_str = boost::lexical_cast<std::string>(endpoint.port);
  int result(0);
  *address_info = SocketGetAddrinfo(address, port_str.c_str(), hints, &result);
  if (result != 0) {
    DLOG(ERROR) << "Incorrect endpoint. " << endpoint.ip << ":" << endpoint.port
                << std::endl;
    *socket_id = UDT::INVALID_SOCK;
    return kInvalidAddress;
  }
  return GetNewSocket(reuse_address, socket_id, *address_info);
}

TransportCondition GetNewSocket(
    bool reuse_address,
    SocketId *socket_id,
    boost::shared_ptr<addrinfo const> address_info) {
  if (socket_id == NULL)
    return kConnectError;

  *socket_id = UDT::socket(address_info->ai_family,
                               address_info->ai_socktype,
                               address_info->ai_protocol);

  if (UDT::INVALID_SOCK == *socket_id) {
    DLOG(ERROR) << "GetNewSocket error: " <<
        UDT::getlasterror().getErrorMessage() << std::endl;
    return kNoSocket;
  }

  // Windows UDP problems fix
#ifdef WIN32
  int mtu(1052);
  UDT::setsockopt(*socket_id, 0, UDT_MSS, &mtu, sizeof(mtu));
#endif
  // UDT_REUSEADDR defaults to true
  if (!reuse_address) {
    UDT::setsockopt(*socket_id, 0, UDT_REUSEADDR, &reuse_address,
                    sizeof(reuse_address));
  }
  return kSuccess;
}

TransportCondition Connect(const SocketId &socket_id,
                           boost::shared_ptr<addrinfo const> peer) {
  if (UDT::ERROR == UDT::connect(socket_id, peer->ai_addr, peer->ai_addrlen)) {
    DLOG(ERROR) << "Connect: " << UDT::getlasterror().getErrorMessage()
                << std::endl;
    UDT::close(socket_id);
    return kConnectError;
  }
  return kSuccess;
}

TransportCondition SetSyncMode(const SocketId &socket_id, bool synchronous) {
  if (UDT::ERROR == UDT::setsockopt(socket_id, 0, UDT_RCVSYN, &synchronous,
                                    sizeof(synchronous))) {
    DLOG(ERROR) << "SetSyncMode " << (synchronous ? "on" : "off") << " UDT_RCVS"
        "YN error: " << UDT::getlasterror().getErrorMessage() << std::endl;
    return kSetOptionFailure;
  }
  if (UDT::ERROR == UDT::setsockopt(socket_id, 0, UDT_SNDSYN, &synchronous,
                                    sizeof(synchronous))) {
    DLOG(ERROR) << "SetSyncMode " << (synchronous ? "on" : "off") << " UDT_SNDS"
        "YN error: " << UDT::getlasterror().getErrorMessage() << std::endl;
    return kSetOptionFailure;
  }
  return kSuccess;
}

}  // namespace udtutils

}  // namespace transport

}  // namespace maidsafe
