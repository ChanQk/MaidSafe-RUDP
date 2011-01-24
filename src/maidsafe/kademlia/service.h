/* Copyright (c) 2009 maidsafe.net limited
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

#ifndef MAIDSAFE_KADEMLIA_SERVICE_H_
#define MAIDSAFE_KADEMLIA_SERVICE_H_

#include <boost/cstdint.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>

#include <memory>
#include <string>
#include <vector>

#include "maidsafe/kademlia/config.h"
#include "maidsafe/kademlia/contact.h"

namespace maidsafe {

namespace kademlia {

class DataStore;
class RoutingTable;
class MessageHandler;

namespace protobuf {
class SignedValue;
class PingRequest;
class PingResponse;
class FindValueRequest;
class FindValueResponse;
class FindNodesRequest;
class FindNodesResponse;
class StoreRequest;
class StoreResponse;
class DeleteRequest;
class DeleteResponse;
class UpdateRequest;
class UpdateResponse;
class DownlistNotification;
}  // namespace protobuf

namespace test_service { class ServicesTest_BEH_KAD_UpdateValue_Test; }

class Service : public boost::enable_shared_from_this<Service> {
 public:
  Service(std::shared_ptr<RoutingTable> routing_table,
          std::shared_ptr<DataStore> datastore,
          AlternativeStorePtr alternative_store,
          SecurifierPtr securifier);
  void ConnectToMessageHandler(std::shared_ptr<MessageHandler> message_handler);
  void Ping(const transport::Info &info,
            const protobuf::PingRequest &request,
            protobuf::PingResponse *response);
  void FindValue(const transport::Info &info,
                 const protobuf::FindValueRequest &request,
                 protobuf::FindValueResponse *response);
  void FindNodes(const transport::Info &info,
                 const protobuf::FindNodesRequest &request,
                 protobuf::FindNodesResponse *response);
  void Store(const transport::Info &info,
             const protobuf::StoreRequest &request,
             protobuf::StoreResponse *response);
  void Delete(const transport::Info &info,
              const protobuf::DeleteRequest &request,
              const std::string &message,
              const std::string &message_signature,
              protobuf::DeleteResponse *response);
  void Update(const transport::Info &info,
              const protobuf::UpdateRequest &request,
              const std::string &message,
              const std::string &message_signature,
              protobuf::UpdateResponse *response);
  void Downlist(const transport::Info &info,
                const protobuf::DownlistNotification &request);
  void set_node_joined(bool joined) { node_joined_ = joined; }
  void set_node_contact(const Contact &contact) { node_contact_ = contact; }
  void set_securifier(SecurifierPtr securifier) { securifier_ = securifier; }
 private:
  friend class test_service::ServicesTest_BEH_KAD_UpdateValue_Test;
  Service(const Service&);
  Service& operator=(const Service&);
  bool StoreValueLocal(const std::string &key,
                       const std::string &value,
                       const boost::int32_t &ttl,
                       bool publish,
                       std::string *serialised_deletion_signature);
  bool StoreValueLocal(const std::string &key,
                       const protobuf::SignedValue &signed_value,
                       const boost::int32_t &ttl,
                       bool publish,
                       std::string *serialised_deletion_signature);
  bool SignedValueHashable(const std::string &key,
                           const protobuf::SignedValue &signed_value);
  bool CanStoreSignedValueHashable(const std::string &key,
                                   const protobuf::SignedValue &signed_value,
                                   bool *hashable);
  std::shared_ptr<RoutingTable> routing_table_;
  std::shared_ptr<DataStore> datastore_;
  AlternativeStorePtr alternative_store_;
  SecurifierPtr securifier_;
  bool node_joined_;
  Contact node_contact_;
};

}  // namespace kademlia

}  // namespace maidsafe

#endif  // MAIDSAFE_KADEMLIA_SERVICE_H_
