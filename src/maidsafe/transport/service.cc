///* Copyright (c) 2010 maidsafe.net limited
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification,
//are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//    * Neither the name of the maidsafe.net limited nor the names of its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
//THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//*/
//
//#include "maidsafe/kademlia/service.h"
//
////#include <boost/compressed_pair.hpp>
////#include <utility>
//#include "maidsafe/common/alternativestore.h"
//#include "maidsafe/common/log.h"
//#include "maidsafe/common/threadpool.h"
//#include "maidsafe/common/validationinterface.h"
////#include "maidsafe/kademlia/rpcs.h"
////#include "maidsafe/kademlia/nodeimpl.h"
////#include "maidsafe/kademlia/datastore.h"
////#include "maidsafe/common/crypto.h"
////#include "maidsafe/common/utils.h"
////#include "maidsafe/kademlia/node-api.h"
////#include "maidsafe/protobuf/signed_kadvalue.pb.h"
////#include "maidsafe/kademlia/nodeid.h"
//#include "maidsafe/protobuf/kademlia.pb.h"
//#include "maidsafe/protobuf/transport_message.pb.h"
//
//namespace maidsafe {
//
//namespace transport {
//
//static void downlist_ping_cb(const std::string&) {}
//
//Service::Service(boost::shared_ptr<transport::Transport> transport,
//                       boost::shared_ptr<RoutingTable> routing_table,
//                       boost::shared_ptr<Threadpool> threadpool,
//                       boost::shared_ptr<DataStore> datastore,
//                       bool using_signatures)
//    : transport_(transport),
//      routing_table_(routing_table),
//      threadpool_(threadpool),
//      datastore_(datastore),
//      node_joined_(false),
//      using_signatures_(using_signatures),
//      node_info_(),
//      alternative_store_(NULL),
//      signature_validator_(NULL) {
//  // Connect to transport signals to process messages received
//  connection_to_message_received_ =
//      transport_->signals()->ConnectOnMessageReceived(boost::bind(
//          &Service::Demux, this, _1, _2, _3));
//}
//
//void Service::Demux(transport::SocketId message_id,
//                       transport::TransportMessage message,
//                       transport::Stats stats) {
//  // parse message, get type and have the service find result.
//  // pass all services to an io_service (threadpool)
//  if (TransportMessage().data().GetExtension<Ping>(message)) {
//  threadpool_->EnqueueTask(boost::bind(&Service::Ping,
//                                      this, _1, _2);
//  } // and the other else if statements - or switch
//}
//
//
//void Service::Ping(transport::SocketId &message_id,
//            const boost::shared_ptr<transport::PingRequest> request) {
//    transport::TransportMessage::Data response;
//    response.SetExtension<ping_response>(set_echo, "pong");
//    response.SetExtension<ping_response>(set_result, true);
//    response.SetExtension<ping_response>(set_node_id, node_info_.node_id());
//   // transport_.send(message_id, response.SerializeToString());
//    // send to transport here, we do not care if it times out or any such thing,
//    //were answering a kademlia request  
//}
//
//void Service::FindNode(google::protobuf::RpcController *controller,
//                          const FindRequest *request, FindResponse *response,
//                          google::protobuf::Closure *done) {
//  if (!node_joined_) {
//    response->set_result(false);
//    done->Run();
//    return;
//  }
//  Contact sender;
//  if (!request->IsInitialized()) {
//    response->set_result(false);
//  } else if (GetSender(request->sender_info(), &sender)) {
//    std::vector<Contact> closest_contacts, exclude_contacts;
//    NodeId key(request->key());
//    if (key.IsValid()) {
//      exclude_contacts.push_back(sender);
//      routing_table_->FindCloseNodes(key, exclude_contacts, &closest_contacts);
//      bool found_node(false);
//      for (unsigned int i = 0; i < closest_contacts.size(); ++i) {
//        std::string contact_str;
//        closest_contacts[i].SerialiseToString(&contact_str);
//        response->add_closest_nodes(contact_str);
//        if (key == closest_contacts[i].node_id())
//          found_node = true;
//      }
//      if (!found_node) {
//        Contact key_node;
//        if (get_contact_(key, &key_node)) {
//          std::string str_key_contact;
//          key_node.SerialiseToString(&str_key_contact);
//          response->add_closest_nodes(str_key_contact);
//        }
//      }
//      response->set_result(true);
//    } else {
//      response->set_result(false);
//    }
//    rpcprotocol::Controller *ctrl = static_cast<rpcprotocol::Controller*>
//                                    (controller);
//    if (ctrl != NULL) {
//      add_contact_(sender, ctrl->rtt(), false);
//    } else {
//      add_contact_(sender, 0.0, false);
//    }
//  } else {
//    response->set_result(false);
//  }
//  response->set_node_id(node_info_.node_id());
//  done->Run();
//}
//
//void Service::FindValue(google::protobuf::RpcController *controller,
//                           const FindRequest *request, FindResponse *response,
//                           google::protobuf::Closure *done) {
//  if (!node_joined_) {
//    response->set_result(false);
//    done->Run();
//    return;
//  }
//  Contact sender;
//  if (!request->IsInitialized()) {
//    response->set_result(false);
//  } else if (GetSender(request->sender_info(), &sender)) {
//    // If the value exists in the alternative store, add our contact details to
//    // field alternative_value_holder.  If not, get the values if present in
//    // this node's data store, otherwise execute find_node for this key.
//    std::string key(request->key());
//    std::vector<std::string> values_str;
//    if (alternative_store_ != NULL) {
//      if (alternative_store_->Has(key)) {
//        *(response->mutable_alternative_value_holder()) = node_info_;
//        response->set_result(true);
//        response->set_node_id(node_info_.node_id());
//        done->Run();
//        return;
//      }
//    }
//    if (pdatastore_->LoadItem(key, &values_str)) {
//      if (using_signatures_) {
//        for (unsigned int i = 0; i < values_str.size(); i++) {
//          SignedValue *signed_value = response->add_signed_values();
//          signed_value->ParseFromString(values_str[i]);
//        }
//      } else {
//        for (unsigned int i = 0; i < values_str.size(); i++)
//          response->add_values(values_str[i]);
//      }
//      response->set_result(true);
//      rpcprotocol::Controller *ctrl = static_cast<rpcprotocol::Controller*>
//        (controller);
//      if (ctrl != NULL) {
//        add_contact_(sender, ctrl->rtt(), false);
//      } else  {
//        add_contact_(sender, 0.0, false);
//      }
//    } else {
//      FindNode(controller, request, response, done);
//      return;
//    }
//  } else {
//    response->set_result(false);
//  }
//  response->set_node_id(node_info_.node_id());
//  done->Run();
//}
//
//void Service::Store(google::protobuf::RpcController *controller,
//                       const StoreRequest *request, StoreResponse *response,
//                       google::protobuf::Closure *done) {
//  DLOG(WARNING) << "Service::Store - " << node_info_.port() << std::endl;
//  if (!node_joined_) {
//    response->set_result(false);
//    done->Run();
//    DLOG(WARNING) << "Not joined? How'd I get the message, then?" << std::endl;
//    return;
//  }
//  Contact sender;
//  rpcprotocol::Controller *ctrl = static_cast<rpcprotocol::Controller*>
//                                  (controller);
//  if (!CheckStoreRequest(request, &sender)) {
//    response->set_result(false);
//  } else if (using_signatures_) {
//    if (signature_validator_ == NULL ||
//        !signature_validator_->ValidateSignerId(
//            request->signed_request().signer_id(),
//            request->signed_request().public_key(),
//            request->signed_request().signed_public_key()) ||
//        !signature_validator_->ValidateRequest(
//            request->signed_request().signed_request(),
//            request->signed_request().public_key(),
//            request->signed_request().signed_public_key(), request->key())) {
//      DLOG(WARNING) << "Failed to validate Store request for kademlia value"
//                    << std::endl;
//      response->set_result(false);
//    } else {
//      StoreValueLocal(request->key(), request->sig_value(), sender,
//                      request->ttl(), request->publish(), response, ctrl);
//    }
//  } else {
//    StoreValueLocal(request->key(), request->value(), sender, request->ttl(),
//                    request->publish(), response, ctrl);
//  }
//  response->set_node_id(node_info_.node_id());
//  done->Run();
//}
//
//void Service::Delete(google::protobuf::RpcController *controller,
//                        const DeleteRequest *request, DeleteResponse *response,
//                        google::protobuf::Closure *done) {
//  // only node with RSAkeys can delete values
//  if (!node_joined_ || !using_signatures_ || signature_validator_ == NULL ||
//      !request->IsInitialized()) {
//    response->set_result(false);
//    done->Run();
//    return;
//  }
//
//  response->set_node_id(node_info_.node_id());
//  // validating request
//  if (signature_validator_ == NULL ||
//        !signature_validator_->ValidateSignerId(
//          request->signed_request().signer_id(),
//          request->signed_request().public_key(),
//          request->signed_request().signed_public_key()) ||
//        !signature_validator_->ValidateRequest(
//          request->signed_request().signed_request(),
//          request->signed_request().public_key(),
//          request->signed_request().signed_public_key(), request->key())) {
//    response->set_result(false);
//    done->Run();
//    return;
//  }
//
//  // only the signer of the value can delete it
//  std::vector<std::string> values_str;
//  if (!pdatastore_->LoadItem(request->key(), &values_str)) {
//    response->set_result(false);
//    done->Run();
//    return;
//  }
//  crypto::Crypto cobj;
//  if (cobj.AsymCheckSig(request->value().value(),
//      request->value().value_signature(),
//      request->signed_request().public_key(), crypto::STRING_STRING)) {
//    Contact sender;
//    if (pdatastore_->MarkForDeletion(request->key(),
//        request->value().SerializeAsString(),
//        request->signed_request().SerializeAsString()) &&
//        GetSender(request->sender_info(), &sender)) {
//      rpcprotocol::Controller *ctrl = static_cast<rpcprotocol::Controller*>
//        (controller);
//      if (ctrl != NULL)
//        add_contact_(sender, ctrl->rtt(), false);
//      else
//        add_contact_(sender, 0.0, false);
//      response->set_result(true);
//      done->Run();
//      return;
//    }
//  }
//  response->set_result(false);
//  done->Run();
//}
//
//void Service::Update(google::protobuf::RpcController *controller,
//                        const UpdateRequest *request,
//                        UpdateResponse *response,
//                        google::protobuf::Closure *done) {
//  // only node with RSAkeys can update values
//  response->set_node_id(node_info_.node_id());
//  response->set_result(false);
//
//  if (!node_joined_ || !using_signatures_ || !request->IsInitialized()) {
//    done->Run();
//#ifdef DEBUG
//    if (!node_joined_)
//      DLOG(WARNING) << "Service::Update - !node_joined_" << std::endl;
//    if (!using_signatures_)
//      DLOG(WARNING) << "Service::Update - !using_signatures_" << std::endl;
//    if (!request->IsInitialized())
//      DLOG(WARNING) << "Service::Update - !request->IsInitialized()" <<
//                       std::endl;
//#endif
//    return;
//  }
//
//  // validating request
//  if (signature_validator_ == NULL ||
//      !signature_validator_->ValidateSignerId(
//          request->request().signer_id(),
//          request->request().public_key(),
//          request->request().signed_public_key()) ||
//       !signature_validator_->ValidateRequest(
//          request->request().signed_request(),
//          request->request().public_key(),
//          request->request().signed_public_key(), request->key())) {
//    done->Run();
//#ifdef DEBUG
//    if (signature_validator_ == NULL)
//      DLOG(WARNING) << "Service::Update - signature_validator_ == NULL" <<
//                       std::endl;
//    if (!signature_validator_->ValidateSignerId(
//          request->request().signer_id(),
//          request->request().public_key(),
//          request->request().signed_public_key()))
//      DLOG(WARNING) << "Service::Update - Failed ValidateSignerId" <<
//                 std::endl;
//    if (!signature_validator_->ValidateRequest(
//          request->request().signed_request(),
//          request->request().public_key(),
//          request->request().signed_public_key(), request->key()))
//      DLOG(WARNING) << "Service::Update - Failed ValidateRequest" <<
//                 std::endl;
//#endif
//    return;
//  }
//
//  // Check the key exists
//  std::vector<std::string> values_str;
//  if (!pdatastore_->LoadItem(request->key(), &values_str)) {
//    done->Run();
//    DLOG(WARNING) << "Service::Update - Didn't find key" << std::endl;
//    return;
//  }
//
//  // Check the value to be updated exists
//  bool found(false);
//  std::string ser_sv(request->old_value().SerializeAsString());
//  for (size_t n = 0; n < values_str.size() && !found; ++n) {
//    if (ser_sv == values_str[n]) {
//      found = true;
//    }
//  }
//
//  if (!found) {
//    done->Run();
//    DLOG(WARNING) << "Service::Update - Didn't find value" << std::endl;
//    return;
//  }
//
//  crypto::Crypto cobj;
//  if (!cobj.AsymCheckSig(request->new_value().value(),
//                         request->new_value().value_signature(),
//                         request->request().public_key(),
//                         crypto::STRING_STRING)) {
//    done->Run();
//    DLOG(WARNING) << "Service::Update - New value doesn't validate" <<
//                     std::endl;
//    return;
//  }
//
//  SignedValue sv;
//  sv.ParseFromString(ser_sv);
//  if (!cobj.AsymCheckSig(sv.value(),
//                         sv.value_signature(),
//                         request->request().public_key(),
//                         crypto::STRING_STRING)) {
//    done->Run();
//    DLOG(WARNING) << "Service::Update - Old value doesn't validate" <<
//                     std::endl;
//    return;
//  }
//
///*******************************************************************************
//This code would check if the current value is hashable, and accept only
//hashable replacement values.
//
////  bool current_hashable(request->key() ==
////                        cobj.Hash(sv.value() + sv.value_signature(), "",
////                                  crypto::STRING_STRING, false));
////  bool new_hashable(request->key() ==
////                    cobj.Hash(request->new_value().value() +
////                                  request->new_value().value_signature(),
////                              "", crypto::STRING_STRING, false));
////  if (current_hashable && !new_hashable && values_str.size() == size_t(1)) {
////    done->Run();
////    DLOG(WARNING) << "Service::Update - Hashable tags don't match" <<
////                     std::endl;
////    return;
////  }
//*******************************************************************************/
//
//  bool new_hashable(request->key() ==
//                    cobj.Hash(request->new_value().value() +
//                                  request->new_value().value_signature(),
//                              "", crypto::STRING_STRING, false));
//  Contact sender;
//  if (!pdatastore_->UpdateItem(request->key(),
//                               request->old_value().SerializeAsString(),
//                               request->new_value().SerializeAsString(),
//                               request->ttl(), new_hashable)) {
//    done->Run();
//    DLOG(WARNING) << "Service::Update - Failed UpdateItem" << std::endl;
//    return;
//  }
//
//  if (GetSender(request->sender_info(), &sender)) {
//    rpcprotocol::Controller *ctrl = static_cast<rpcprotocol::Controller*>
//                                    (controller);
//    if (ctrl != NULL)
//      add_contact_(sender, ctrl->rtt(), false);
//    else
//      add_contact_(sender, 0.0, false);
//    response->set_result(true);
//  } else {
//    DLOG(WARNING) << "Service::Update - Failed to add_contact_" << std::endl;
//  }
//
//  done->Run();
//}
//
//void Service::Downlist(google::protobuf::RpcController *controller,
//                          const DownlistRequest *request,
//                          DownlistResponse *response,
//                          google::protobuf::Closure *done) {
//  if (!node_joined_) {
//    response->set_result(false);
//    done->Run();
//    return;
//  }
//  Contact sender;
//  if (!request->IsInitialized()) {
//    response->set_result(false);
//  } else if (GetSender(request->sender_info(), &sender)) {
//    for (int i = 0; i < request->downlist_size(); ++i) {
//      Contact dead_node;
//      if (!dead_node.ParseFromString(request->downlist(i)))
//        continue;
//    // A sophisticated attacker possibly send a random downlist. We only verify
//    // the offline status of the nodes in our routing table.
//      Contact contact_to_ping;
//      response->set_result(true);
//      if (get_contact_(dead_node.node_id(), &contact_to_ping)) {
//        ping_(dead_node, boost::bind(&downlist_ping_cb, _1));
//      }
//    }
//    rpcprotocol::Controller *ctrl = static_cast<rpcprotocol::Controller*>
//        (controller);
//    if (ctrl != NULL) {
//      add_contact_(sender, ctrl->rtt(), false);
//    } else {
//      add_contact_(sender, 0.0, false);
//    }
//  } else {
//    response->set_result(false);
//  }
//  response->set_node_id(node_info_.node_id());
//  done->Run();
//}
//
//bool Service::GetSender(const ContactInfo &sender_info, Contact *sender) {
//  std::string ser_info(sender_info.SerializeAsString());
//  return sender->ParseFromString(ser_info);
//}
//
//bool Service::CheckStoreRequest(const StoreRequest *request,
//                                   Contact *sender) {
//  if (!request->IsInitialized())
//    return false;
//  if (using_signatures_) {
//    if (!request->has_signed_request() || !request->has_sig_value())
//      return false;
//  } else {
//    if (!request->has_value())
//      return false;
//  }
//  return GetSender(request->sender_info(), sender);
//}
//
//void Service::StoreValueLocal(const std::string &key,
//                                 const std::string &value, Contact sender,
//                                 const boost::int32_t &ttl,
//                                 const bool &publish, StoreResponse *response,
//                                 rpcprotocol::Controller *ctrl) {
//  bool result;
//  if (publish) {
//    result = pdatastore_->StoreItem(key, value, ttl, false);
//  } else {
//    std::string ser_del_request;
//    result = pdatastore_->RefreshItem(key, value, &ser_del_request);
//    if (!result && ser_del_request.empty()) {
//      result = pdatastore_->StoreItem(key, value, ttl, false);
//    } else if (!result && !ser_del_request.empty()) {
//      SignedRequest *req = response->mutable_signed_request();
//      req->ParseFromString(ser_del_request);
//    }
//  }
//  if (result) {
//    response->set_result(true);
//    if (ctrl != NULL) {
//      add_contact_(sender, ctrl->rtt(), false);
//    } else {
//      add_contact_(sender, 0.0, false);
//    }
//  } else {
//    response->set_result(false);
//  }
//}
//
//void Service::StoreValueLocal(const std::string &key,
//                                 const SignedValue &value, Contact sender,
//                                 const boost::int32_t &ttl, const bool &publish,
//                                 StoreResponse *response,
//                                 rpcprotocol::Controller *ctrl) {
//  bool result, hashable;
//  std::string ser_value(value.value() + value.value_signature());
//  if (publish) {
//    if (CanStoreSignedValueHashable(key, ser_value, &hashable)) {
//      result = pdatastore_->StoreItem(key, value.SerializeAsString(), ttl,
//                                      hashable);
//      if (!result) {
//        DLOG(WARNING) << "pdatastore_->StoreItem 1 Failed.";
//      }
//    } else {
//      DLOG(WARNING) << "CanStoreSignedValueHashable Failed.";
//      result = false;
//    }
//  } else {
//    std::string ser_del_request;
//    result = pdatastore_->RefreshItem(key, value.SerializeAsString(),
//                                      &ser_del_request);
//
//    if (!result && CanStoreSignedValueHashable(key, ser_value, &hashable) &&
//        ser_del_request.empty()) {
//      result = pdatastore_->StoreItem(key, value.SerializeAsString(), ttl,
//                                      hashable);
//      if (!result)
//        DLOG(WARNING) << "pdatastore_->StoreItem 2 Failed.";
//    } else if (!result && !ser_del_request.empty()) {
//      SignedRequest *req = response->mutable_signed_request();
//      req->ParseFromString(ser_del_request);
//      DLOG(WARNING) << "Weird Fail. - adding signed req to resp." << std::endl;
//    } else if (!result) {
//      DLOG(WARNING) << "pdatastore_->RefreshItem Failed.";
//    }
//  }
//  if (result) {
//    response->set_result(true);
//    if (ctrl != NULL) {
//      add_contact_(sender, ctrl->rtt(), false);
//    } else {
//      add_contact_(sender, 0.0, false);
//    }
//  } else {
//    response->set_result(false);
//  }
//}
//
//bool Service::CanStoreSignedValueHashable(const std::string &key,
//                                             const std::string &value,
//                                             bool *hashable) {
//  std::vector< std::pair<std::string, bool> > attr;
//  attr = pdatastore_->LoadKeyAppendableAttr(key);
//  *hashable = false;
//  if (attr.empty()) {
//    crypto::Crypto cobj;
//    cobj.set_hash_algorithm(crypto::SHA_512);
//    if (key == cobj.Hash(value, "", crypto::STRING_STRING, false))
//      *hashable = true;
//  } else if (attr.size() == 1) {
//    *hashable = attr[0].second;
//    if (*hashable && value != attr[0].first) {
//      return false;
//    }
//  }
//  return true;
//}
//
//}  // namespace transport
//
//}  // namespace maidsafe
//