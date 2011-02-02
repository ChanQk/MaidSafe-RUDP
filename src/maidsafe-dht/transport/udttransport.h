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

/*******************************************************************************
 * NOTE: This header is unlikely to have any breaking changes applied.         *
 *       However, it should not be regarded as finalised until this notice is  *
 *       removed.                                                              *
 ******************************************************************************/
/*
* TODO
* 1:  Allow Listening ports to be closed individually and as a group
* 2:  Create managed connection interface
* 3:  Add an Open method for rendezvous connections
* 4:  Create a ping at network level (in UDT this is a connect)
* 5:  Use managed connections for rendezvous
* 6:  Add tcp listen capability, may be another transport
* 7:  Provide a brodcast tcp method " " " " "
* 8:  When a node can it will start a tcp listener on 80 and 443 and add this
*     to the contact tuple (prononced toople apparently :-) )
* 9:  Thread send including connect
* 10  Use thread pool
* 11: On thread pool filling up move all incoming connecitons to an async
*     connection method until a thread becomes available.
* 12: Complete NAT traversal management (use upnp, nat-pmp and hole punching)
*     allong prioratising of method type.
* 13: Use TCP to beackon on port 5483 when contact with kademlia network lost
* 14: Profile profile and profile. The send recive test should be under 100ms
*     preferrably less than 25ms.
* 15: Decide on how / when to fire the Stats signals
* 16: Provide channel level encryption (diffie Hellman -> AES xfer)
*/

#ifndef MAIDSAFE_DHT_TRANSPORT_UDTTRANSPORT_H_
#define MAIDSAFE_DHT_TRANSPORT_UDTTRANSPORT_H_

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "boost/asio/deadline_timer.hpp"
#include "boost/cstdint.hpp"
#include "boost/detail/atomic_count.hpp"
#include "maidsafe-dht/transport/transport.h"

namespace maidsafe {

namespace transport {

struct NatDetails {
  NatDetails()
      : nat_type(kNotConnected),
        external_endpoint(),
        rendezvous_endpoint(),
        local_endpoints() {}
  NatType nat_type;
  Endpoint external_endpoint, rendezvous_endpoint;
  // These endpoints refer to the ones read from the interfaces
  std::vector<Endpoint> local_endpoints;
};

namespace test {
class TestNatTraversal;
class TestNatTraversal_BEH_UDT_DirectlyConnected_Test;
}  // namespace test

typedef int SocketId;

const Timeout kAddManagedEndpointTimeout(bptime::seconds(10));
const size_t kMaxUnusedSocketsCount(10);
const int kManagedSocketBufferSize(200);  // bytes

class UdtTransport : public Transport,
                     public std::enable_shared_from_this<UdtTransport> {
 public:
  explicit UdtTransport(std::shared_ptr<boost::asio::io_service> asio_service);
  UdtTransport(std::shared_ptr<boost::asio::io_service> asio_service,
               std::vector<Endpoint> nat_detection_endpoints);
  virtual ~UdtTransport();
  static void CleanUp();
  virtual TransportCondition StartListening(const Endpoint &endpoint);
  virtual void StopListening();
  virtual void Send(const std::string &data,
                    const Endpoint &endpoint,
                    const Timeout &timeout);
  virtual void SendStream(const std::istream &data, const Endpoint &endpoint);
  void SendToPortRestricted(const std::string &data,
                            const Endpoint &endpoint,
                            const Endpoint &rendezvous_endpoint,
                            const Timeout &timeout);



/*
  // Closes all managed connections and stops accepting new incoming ones.
  void StopManagedEndpoints();
  // Allows new incoming managed connections after StopManagedEndpoints has
  // been called.
  void ReAllowIncomingManagedEndpoints();
  // Adds an endpoint that is checked at frequency milliseconds, or which keeps
  // alive the connection if frequency == 0.  Checking persists until
  // RemoveManagedEndpoint called, or endpoint is unavailable.
  // Return value is the socket id or -1 on error.  For frequency == 0 (implies
  // stay connected) the ManagedEndpointId can be used as the SocketId for
  // sending further messages.  For frequency > 0, new connections are
  // regularly made and broken, so ManagedEndpointId cannot be used as SocketId.
  // On failure to connect, retry_count further attempts at retry_frequency (ms)
  // are performed before failure.
  ManagedEndpointId AddManagedEndpoint(
      const IP &remote_ip,
      const Port &remote_port,
      const IP &rendezvous_ip,
      const Port &rendezvous_port,
      const std::string &our_identifier,
      const boost::uint16_t &frequency,
      const boost::uint16_t &retry_count,
      const boost::uint16_t &retry_frequency);
  bool RemoveManagedEndpoint(const ManagedEndpointId &managed_endpoint_id);*/
  friend class UdtConnection;
  friend class test::TestNatTraversal;
  friend class test::TestNatTraversal_BEH_UDT_DirectlyConnected_Test;




 private:
  UdtTransport& operator=(const UdtTransport&);
  UdtTransport(const UdtTransport&);
  TransportCondition DoStartListening(const Endpoint &endpoint,
                                      bool managed_endpoint_listener);
  void AcceptConnection(bool managed_endpoint_accept);
  void DoSend(const std::string &data,
              const SocketId &socket_id,
              const Timeout &timeout);

  // NAT Detection methods
  void DetectNat();



  // Managed Endpoint methods
  TransportCondition StartManagedEndpointListener(
      const SocketId &initial_peer_socket_id,
      std::shared_ptr<addrinfo const> peer);
  TransportCondition SetManagedSocketOptions(const SocketId &socket_id);
  SocketId GetNewManagedEndpointSocket(const IP &remote_ip,
                                       const Port &remote_port,
                                       const IP &rendezvous_ip,
                                       const Port &rendezvous_port);
  void CheckManagedSockets();
/*  void HandleManagedSocketRequest(const SocketId &socket_id,
                                  const ManagedEndpointMessage &request);
  void HandleManagedSocketResponse(const SocketId &managed_socket_id,
                                   const ManagedEndpointMessage &response);
  // This is only meant to be used as a predicate where
  // managed_endpoint_sockets_mutex_ is already locked.
  bool PendingManagedSocketReplied(const SocketId &socket_id);
  TransportCondition TryRendezvous(const IP &ip, const Port &port,
                                   SocketId *rendezvous_socket_id);
  void PerformNatDetection(const SocketId &socket_id,
                           const NatDetection &nat_detection_message);
  void ReportRendezvousResult(const SocketId &socket_id,
                              const IP &connection_node_ip,
                              const Port &connection_node_port);*/

  SocketId listening_socket_id_, managed_endpoint_listening_socket_id_;
  Port managed_endpoint_listening_port_;
  boost::asio::deadline_timer timer_;
  boost::detail::atomic_count accepted_connection_count_;
  static NatDetails nat_details_;
  std::vector<Endpoint> nat_detection_endpoints_;




//  std::vector<SocketId> managed_endpoint_sockets_;
//  std::map<SocketId, SocketId> pending_managed_endpoint_sockets_;
//  bool stop_managed_endpoints_, managed_endpoints_stopped_;
//  boost::mutex managed_endpoint_sockets_mutex_;
//  boost::condition_variable managed_endpoints_cond_var_;
  std::shared_ptr<addrinfo const> managed_endpoint_listening_addrinfo_;
//  std::shared_ptr<boost::thread> check_connections_;
//  boost::thread nat_detection_thread_;
};

}  // namespace transport

}  // namespace maidsafe

#endif  // MAIDSAFE_DHT_TRANSPORT_UDTTRANSPORT_H_

