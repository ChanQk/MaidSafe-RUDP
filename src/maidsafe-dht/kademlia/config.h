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

#ifndef MAIDSAFE_DHT_KADEMLIA_CONFIG_H_
#define MAIDSAFE_DHT_KADEMLIA_CONFIG_H_

#include <memory>
#include <string>
#include <vector>

#include "boost/asio/ip/address.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/cstdint.hpp"
#include "boost/date_time/posix_time/posix_time_duration.hpp"
#include "boost/function.hpp"
#ifdef __MSVC__
#pragma warning(push)
#pragma warning(disable:4512)
#endif
#include "boost/signals2/signal.hpp"
#ifdef __MSVC__
#pragma warning(pop)
#endif

namespace maidsafe {

class Securifier;

namespace transport {
class Transport;
struct Endpoint;
struct Info;
}  // namespace transport

namespace kademlia {

class AlternativeStore;
class NodeId;
class Contact;
class MessageHandler;

enum OnlineStatus { kOffline, kOnline, kAttemptingConnect };

typedef std::shared_ptr<boost::signals2::signal<void(OnlineStatus)>>
    OnOnlineStatusChangePtr;

// Functor for use in Node::Join, Store, Delete and Update.  Parameter is the
// return code.
typedef boost::function<void(int)> JoinFunctor, StoreFunctor, DeleteFunctor,  // NOLINT (Fraser)
                                   UpdateFunctor;

// Functor for use in Node::FindValue.  Parameters in order are: return code,
// value(s) if found, k closest nodes if value not found, contact details of
// node holding value in its alternative_store, and contact details of node
// needing a cache copy of the values.
typedef boost::function<void(int,
                             std::vector<std::string>,
                             std::vector<Contact>,
                             Contact,
                             Contact)> FindValueFunctor;

// Functor for use in Node::FindNodes.  Parameters in order are: return code,
// k closest nodes.
typedef boost::function<void(int, std::vector<Contact>)> FindNodesFunctor;

// Functor for use in Node::GetContact.  Parameters in order are: return code,
// node's contact details.
typedef boost::function<void(int, Contact)> GetContactFunctor;


typedef NodeId Key;
typedef boost::asio::ip::address IP;
typedef boost::uint16_t Port;


typedef std::shared_ptr<boost::asio::io_service> IoServicePtr;
typedef std::shared_ptr<transport::Transport> TransportPtr;
typedef std::shared_ptr<MessageHandler> MessageHandlerPtr;
typedef std::shared_ptr<Securifier> SecurifierPtr;
typedef std::shared_ptr<AlternativeStore> AlternativeStorePtr;
typedef std::shared_ptr<transport::Info> RankInfoPtr;


// The size of DHT keys and node IDs in bytes.
const boost::uint16_t kKeySizeBytes(64);

// The mean time between refreshes
const boost::posix_time::seconds kMeanRefreshInterval(1800);

// The ratio of k successful individual kad store RPCs to yield overall success.
const double kMinSuccessfulPecentageStore(0.75);

// The ratio of k successful individual kad delete RPCs to yield overall success
const double kMinSuccessfulPecentageDelete(0.75);

// The ratio of k successful individual kad update RPCs to yield overall success
const double kMinSuccessfulPecentageUpdate(0.75);

// The number of failed RPCs tolerated before a contact is removed from the
// routing table.
const boost::uint16_t kFailedRpcTolerance(2);

}  // namespace kademlia

}  // namespace maidsafe

#endif  // MAIDSAFE_DHT_KADEMLIA_CONFIG_H_
