/* Copyright (c) 2009 maidsafe.net limited
All rights reserved.
File Created: 2011/03/16
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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "boost/thread.hpp"
#include "boost/thread/mutex.hpp"
#include "maidsafe-dht/kademlia/message_handler.h"
#include "maidsafe-dht/transport/message_handler.h"
#include "maidsafe-dht/kademlia/message_handler.cc"
#include "maidsafe-dht/kademlia/utils.h"
#include "maidsafe-dht/kademlia/rpcs.h"
#include "maidsafe-dht/kademlia/securifier.h"
#include "maidsafe-dht/kademlia/node_id.h"
#include "maidsafe-dht/transport/transport.pb.h"

#include "maidsafe/common/crypto.h"

namespace maidsafe {

namespace kademlia {

namespace test {

namespace mock_rpcs {
  const boost::uint16_t kFailureTolerance = 2;
}  //  namespace mock_rpcs

// Mock the TcpTransport class
class MockTransport : public transport::Transport {
 public:
  MockTransport(boost::asio::io_service &asio_service, const int &repeat_factor)
    : repeat_factor_(repeat_factor),
      Transport(asio_service) {}
  ~MockTransport() {}
  virtual transport::TransportCondition StartListening(
      const transport::Endpoint &endpoint) {
    return transport::kSuccess;
  }
  virtual void StopListening() {}
  virtual void Send(const std::string &data,
                    const transport::Endpoint &endpoint,
                    const transport::Timeout &timeout) {
    transport::Info info;
    transport::Timeout response_timeout(transport::kImmediateTimeout);
    std::string response("response");
    if (repeat_factor_ < mock_rpcs::kFailureTolerance) {
      ++repeat_factor_;
      (*on_error())(transport::kError);
    } else {
      (*on_message_received())(data, info, &response, &response_timeout);
    }
  }
 protected:
  boost::uint16_t repeat_factor_;
};

class MockMessageHandler : public MessageHandler {
 public:
  MockMessageHandler(SecurifierPtr securifier, const int &request_type,
                     const int &result_type)
    : securifier_(securifier),
      request_type_(request_type),
      result_type_(result_type),
      MessageHandler(securifier) {}
  void OnMessageReceived(const std::string &request,
                         const transport::Info &info,
                         std::string *response,
                         transport::Timeout *timeout) {
    ProcessSerialisedMessage(request_type_, request, kNone, "",
                             info, response, timeout);
  }

  void ProcessSerialisedMessage(
    const int &message_type,
    const std::string &payload,
    const SecurityType &security_type,
    const std::string &message_signature,
    const transport::Info &info,
    std::string *message_response,
    transport::Timeout* timeout) {
  message_response->clear();
  *timeout = transport::kImmediateTimeout;

  switch (message_type) {
    case kademlia::kPingRequest: {
      protobuf::PingResponse response;
      switch (result_type_) {
        case 1: {
          response.set_echo("pong");
          break;
        }
        case 2: {
          response.set_echo("");
          break;
        }
        default:
          break;
      }
      (*on_ping_response())(info, response);
      break;
    }
    case kFindValueRequest: {
      protobuf::FindValueResponse response;
      switch (result_type_) {
        case 1: {
          protobuf::SignedValue value;
          value.set_value("value");
          value.set_signature("signature");
          Contact contact, alternative_value_holder;
          response.set_result(true);
          *response.add_closest_nodes() = ToProtobuf(contact);
          *response.add_signed_values() = value;
          *response.mutable_alternative_value_holder() =
              ToProtobuf(alternative_value_holder);
          break;
        }
        case 2: {
          response.set_result(false);
          break;
        }
        default:
          break;
      }
      (*on_find_value_response())(info, response);
      break;
    }
    case kFindNodesRequest: {
      protobuf::FindNodesResponse response;
      switch (result_type_) {
        case 1: {
          Contact contact;
          response.set_result(true);
          *response.add_closest_nodes() = ToProtobuf(contact);
          break;
        }
        case 2: {
          response.set_result(false);
          break;
        }
        default:
          break;
      }
      (*on_find_nodes_response())(info, response);
      break;
    }
    case kStoreRequest: {
      protobuf::StoreResponse response;
      switch (result_type_) {
        case 1: {
          response.set_result(true);
          break;
        }
        case 2: {
          response.set_result(false);
          break;
        }
        default:
          break;
      }
      (*on_store_response())(info, response);
      break;
    }
    case kStoreRefreshRequest: {
      protobuf::StoreRefreshResponse response;
      switch (result_type_) {
        case 1: {
          response.set_result(true);
          break;
        }
        case 2: {
          response.set_result(false);
          break;
        }
        default:
          break;
      }
      (*on_store_refresh_response())(info, response);
      break;
    }
    case kDeleteRequest: {
      protobuf::DeleteResponse response;
      switch (result_type_) {
        case 1: {
          response.set_result(true);
          break;
        }
        case 2: {
          response.set_result(false);
          break;
        }
        default:
          break;
      }
      (*on_delete_response())(info, response);
      break;
    }
    case kDeleteRefreshRequest: {
      protobuf::DeleteRefreshResponse response;
      switch (result_type_) {
        case 1: {
          response.set_result(true);
          break;
        }
        case 2: {
          response.set_result(false);
          break;
        }
        default:
          break;
      }
      (*on_delete_refresh_response())(info, response);
      break;
    }
    case kDownlistNotification: {
      protobuf::DownlistNotification request;
      transport::protobuf::WrapperMessage wrapper;
      std::string serialised_message(payload.substr(1));
      wrapper.ParseFromString(serialised_message);
      if (request.ParseFromString(wrapper.payload()))
        ASSERT_EQ(size_t(1), request.node_ids_size());
      break;
    }
    default:
      break;
  }
}
 protected:
  SecurifierPtr securifier_;
  int request_type_;
  int result_type_;
};

typedef std::shared_ptr<MockMessageHandler> MockMessageHandlerPtr;

class MockRpcs : public Rpcs {
 public:
  MockRpcs(IoServicePtr asio_service, SecurifierPtr securifier,
           const int &request_type, const int &repeat_factor,
           const int &result_type)
    : asio_service_(asio_service), securifier_(securifier),
      request_type_(request_type),
      Rpcs(asio_service, securifier),
      result_type_(result_type), repeat_factor_(repeat_factor) {}
  ~MockRpcs() {}
  MOCK_METHOD2(Prepare, Rpcs::ConnectedObjects(TransportType type,
                                               SecurifierPtr securifier));
  ConnectedObjects MockPrepare(TransportType type,
                               SecurifierPtr securifier) {
    std::shared_ptr<MockTransport> transport;
    transport.reset(new MockTransport(*asio_service_, repeat_factor_));
    MockMessageHandlerPtr message_handler(new MockMessageHandler(
        securifier, request_type_, result_type_));
    bs2::connection on_recv_con = transport->on_message_received()->connect(
        boost::bind(&MockMessageHandler::OnMessageReceived,
                    message_handler.get(), _1, _2, _3, _4));
    bs2::connection on_err_con = transport->on_error()->connect(
        boost::bind(&MockMessageHandler::OnError, message_handler.get(), _1));
    return boost::make_tuple(transport, message_handler, on_recv_con,
                             on_err_con);
  }
 protected:
  IoServicePtr asio_service_;
  SecurifierPtr securifier_;
  int request_type_, result_type_, repeat_factor_;
};

class MockRpcsTest : public testing::Test {
 public:
  MockRpcsTest()
          : asio_service_(new boost::asio::io_service),
            securifier_(),
            peer_(ComposeContact(NodeId(NodeId::kRandomId), 6789)) {
    securifier_.reset(new Securifier("", "", ""));
  }

  ~MockRpcsTest() {}

  void SetUp() {}

  Contact ComposeContact(const NodeId& node_id, boost::uint16_t port) {
    std::string ip("127.0.0.1");
    std::vector<transport::Endpoint> local_endpoints;
    transport::Endpoint end_point(ip, port);
    local_endpoints.push_back(end_point);
    Contact contact(node_id, end_point, local_endpoints, end_point, false,
                    false, "", "", "");
    return contact;
  }

  void Callback(RankInfoPtr rank_info, const int &result, bool *b,
                boost::mutex *m, int *query_result) {
    boost::mutex::scoped_lock loch_lomond(*m);
    *b = true;
    *query_result = result;
  }
  void FindNodesCallback(RankInfoPtr rank_info, const int &result,
                         std::vector<Contact> contacts, bool *b,
                         boost::mutex *m, int *query_result) {
    Callback(rank_info, result, b, m, query_result);
  }
  void FindValueCallback(RankInfoPtr rank_info, const int &result,
                         const std::vector<std::string> &values,
                         const std::vector<Contact> &contacts, const Contact
                         &contact, bool *b, boost::mutex *m,
                         int *query_result) {
    Callback(rank_info, result, b, m, query_result);
  }
  protected:
  IoServicePtr asio_service_;
  SecurifierPtr securifier_;
  Contact peer_;
};

TEST_F(MockRpcsTest, BEH_KAD_Rpcs_Ping) {
  int repeat_factor(1);
  int result_type(1);
  for (int i = 0; i < 5; ++i) {
    boost::shared_ptr<MockRpcs> rpcs_(new MockRpcs(asio_service_, securifier_,
                                                   kPingRequest, repeat_factor,
                                                   result_type));
    bool b(false), b2(false);
    int result(999);
    boost::mutex m;
    EXPECT_CALL(*rpcs_, Prepare(testing::_, testing::_))
        .WillOnce(testing::WithArgs<0, 1>(testing::Invoke(boost::bind(
        &MockRpcs::MockPrepare, rpcs_.get(), _1, _2))));

    Rpcs::PingFunctor pf = boost::bind(&MockRpcsTest::Callback, this, _1, _2,
                                       &b, &m, &result);
    boost::thread t1(&Rpcs::Ping, rpcs_, securifier_, peer_, pf, kTcp);
    while (!b2) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      boost::mutex::scoped_lock loch_lomond(m);
      b2 = b;
    }
    switch (i) {
      case 0: {
        ASSERT_EQ(transport::kSuccess, result);
        result_type = 2;
        break;
      }
      case 1: {
        ASSERT_EQ(transport::kError, result);
        result_type = 1;
        repeat_factor = mock_rpcs::kFailureTolerance;
        break;
      }
      case 2: {
        ASSERT_EQ(transport::kSuccess, result);
        repeat_factor = 0;
        break;
      }
      case 3: {
        ASSERT_EQ(transport::kError, result);
        repeat_factor = mock_rpcs::kFailureTolerance - 1;
        break;
      }
      case 4: {
        ASSERT_EQ(transport::kSuccess, result);
        break;
      }
      default:
        break;
    }
  }
}

TEST_F(MockRpcsTest, BEH_KAD_Rpcs_Store) {
  int repeat_factor(1);
  int result_type(1);
  for (int i = 0; i < 5; ++i) {
    boost::shared_ptr<MockRpcs> rpcs_(new MockRpcs(asio_service_, securifier_,
                                                   kStoreRequest, repeat_factor,
                                                   result_type));
    bool b(false), b2(false);
    int result(999);
    boost::mutex m;
    EXPECT_CALL(*rpcs_, Prepare(testing::_, testing::_))
        .WillOnce(testing::WithArgs<0, 1>(testing::Invoke(boost::bind(
        &MockRpcs::MockPrepare, rpcs_.get(), _1, _2))));

    Rpcs::StoreFunctor sf = boost::bind(&MockRpcsTest::Callback, this, _1, _2,
                                       &b, &m, &result);
    boost::thread t1(&Rpcs::Store, rpcs_, NodeId(NodeId::kRandomId), "",
                     "", boost::posix_time::seconds(1), securifier_,
                     peer_, sf, kTcp);
    while (!b2) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      boost::mutex::scoped_lock loch_lomond(m);
      b2 = b;
    }
    switch (i) {
      case 0: {
        ASSERT_EQ(transport::kSuccess, result);
        result_type = 2;
        break;
      }
      case 1: {
        ASSERT_EQ(transport::kError, result);
        result_type = 1;
        repeat_factor = mock_rpcs::kFailureTolerance;
        break;
      }
      case 2: {
        ASSERT_EQ(transport::kSuccess, result);
        repeat_factor = 0;
        break;
      }
      case 3: {
        ASSERT_EQ(transport::kError, result);
        repeat_factor = mock_rpcs::kFailureTolerance - 1;
        break;
      }
      case 4: {
        ASSERT_EQ(transport::kSuccess, result);
        break;
      }
      default:
        break;
    }
  }
}

TEST_F(MockRpcsTest, BEH_KAD_Rpcs_StoreRefresh) {
  int repeat_factor(1);
  int result_type(1);
  for (int i = 0; i < 5; ++i) {
    boost::shared_ptr<MockRpcs> rpcs_(new MockRpcs(asio_service_, securifier_,
                                                   kStoreRefreshRequest,
                                                   repeat_factor,
                                                   result_type));
    bool b(false), b2(false);
    int result(999);
    boost::mutex m;
    EXPECT_CALL(*rpcs_, Prepare(testing::_, testing::_))
        .WillOnce(testing::WithArgs<0, 1>(testing::Invoke(boost::bind(
        &MockRpcs::MockPrepare, rpcs_.get(), _1, _2))));

    Rpcs::StoreRefreshFunctor srf = boost::bind(&MockRpcsTest::Callback, this,
                                                _1, _2, &b, &m, &result);
    boost::thread t1(&Rpcs::StoreRefresh, rpcs_, "", "", securifier_, peer_,
                     srf, kTcp);
    while (!b2) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      boost::mutex::scoped_lock loch_lomond(m);
      b2 = b;
    }
    switch (i) {
      case 0: {
        ASSERT_EQ(transport::kSuccess, result);
        result_type = 2;
        break;
      }
      case 1: {
        ASSERT_EQ(transport::kError, result);
        result_type = 1;
        repeat_factor = mock_rpcs::kFailureTolerance;
        break;
      }
      case 2: {
        ASSERT_EQ(transport::kSuccess, result);
        repeat_factor = 0;
        break;
      }
      case 3: {
        ASSERT_EQ(transport::kError, result);
        repeat_factor = mock_rpcs::kFailureTolerance - 1;
        break;
      }
      case 4: {
        ASSERT_EQ(transport::kSuccess, result);
        break;
      }
      default:
        break;
    }
  }
}

TEST_F(MockRpcsTest, BEH_KAD_Rpcs_Delete) {
  int repeat_factor(1);
  int result_type(1);
  for (int i = 0; i < 5; ++i) {
    boost::shared_ptr<MockRpcs> rpcs_(new MockRpcs(asio_service_, securifier_,
                                                   kDeleteRequest,
                                                   repeat_factor,
                                                   result_type));
    bool b(false), b2(false);
    int result(999);
    boost::mutex m;
    EXPECT_CALL(*rpcs_, Prepare(testing::_, testing::_))
        .WillOnce(testing::WithArgs<0, 1>(testing::Invoke(boost::bind(
        &MockRpcs::MockPrepare, rpcs_.get(), _1, _2))));

    Rpcs::DeleteFunctor df = boost::bind(&MockRpcsTest::Callback, this, _1, _2,
                                         &b, &m, &result);
    boost::thread t1(&Rpcs::Delete, rpcs_, NodeId(NodeId::kRandomId), "",
                     "", securifier_, peer_, df, kTcp);
    while (!b2) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      boost::mutex::scoped_lock loch_lomond(m);
      b2 = b;
    }
    switch (i) {
      case 0: {
        ASSERT_EQ(transport::kSuccess, result);
        result_type = 2;
        break;
      }
      case 1: {
        ASSERT_EQ(transport::kError, result);
        result_type = 1;
        repeat_factor = mock_rpcs::kFailureTolerance;
        break;
      }
      case 2: {
        ASSERT_EQ(transport::kSuccess, result);
        repeat_factor = 0;
        break;
      }
      case 3: {
        ASSERT_EQ(transport::kError, result);
        repeat_factor = mock_rpcs::kFailureTolerance - 1;
        break;
      }
      case 4: {
        ASSERT_EQ(transport::kSuccess, result);
        break;
      }
      default:
        break;
    }
  }
}

TEST_F(MockRpcsTest, BEH_KAD_Rpcs_DeleteRefresh) {
  int repeat_factor(1);
  int result_type(1);
  for (int i = 0; i < 5; ++i) {
    boost::shared_ptr<MockRpcs> rpcs_(new MockRpcs(asio_service_, securifier_,
                                                   kDeleteRefreshRequest,
                                                   repeat_factor,
                                                   result_type));
    bool b(false), b2(false);
    int result(999);
    boost::mutex m;
    EXPECT_CALL(*rpcs_, Prepare(testing::_, testing::_))
        .WillOnce(testing::WithArgs<0, 1>(testing::Invoke(boost::bind(
        &MockRpcs::MockPrepare, rpcs_.get(), _1, _2))));

    Rpcs::DeleteRefreshFunctor drf = boost::bind(&MockRpcsTest::Callback, this,
                                                 _1, _2, &b, &m, &result);
    boost::thread t1(&Rpcs::DeleteRefresh, rpcs_, "", "", securifier_, peer_,
                     drf, kTcp);
    while (!b2) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      boost::mutex::scoped_lock loch_lomond(m);
      b2 = b;
    }
    switch (i) {
      case 0: {
        ASSERT_EQ(transport::kSuccess, result);
        result_type = 2;
        break;
      }
      case 1: {
        ASSERT_EQ(transport::kError, result);
        result_type = 1;
        repeat_factor = mock_rpcs::kFailureTolerance;
        break;
      }
      case 2: {
        ASSERT_EQ(transport::kSuccess, result);
        repeat_factor = 0;
        break;
      }
      case 3: {
        ASSERT_EQ(transport::kError, result);
        repeat_factor = mock_rpcs::kFailureTolerance - 1;
        break;
      }
      case 4: {
        ASSERT_EQ(transport::kSuccess, result);
        break;
      }
      default:
        break;
    }
  }
}

TEST_F(MockRpcsTest, BEH_KAD_Rpcs_FindNodes) {
  int repeat_factor(1);
  int result_type(1);
  for (int i = 0; i < 5; ++i) {
    boost::shared_ptr<MockRpcs> rpcs_(new MockRpcs(asio_service_, securifier_,
                                                   kFindNodesRequest,
                                                   repeat_factor,
                                                   result_type));
    bool b(false), b2(false);
    int result(999);
    boost::mutex m;
    EXPECT_CALL(*rpcs_, Prepare(testing::_, testing::_))
        .WillOnce(testing::WithArgs<0, 1>(testing::Invoke(boost::bind(
        &MockRpcs::MockPrepare, rpcs_.get(), _1, _2))));

    Rpcs::FindNodesFunctor fnf = boost::bind(&MockRpcsTest::FindNodesCallback,
                                             this, _1, _2, _3, &b, &m, &result);

    boost::thread t1(&Rpcs::FindNodes, rpcs_, NodeId(NodeId::kRandomId),
                     securifier_, peer_, fnf, kTcp);
    while (!b2) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      boost::mutex::scoped_lock loch_lomond(m);
      b2 = b;
    }
    switch (i) {
      case 0: {
        ASSERT_EQ(transport::kSuccess, result);
        result_type = 2;
        break;
      }
      case 1: {
        ASSERT_EQ(transport::kError, result);
        result_type = 1;
        repeat_factor = mock_rpcs::kFailureTolerance;
        break;
      }
      case 2: {
        ASSERT_EQ(transport::kSuccess, result);
        repeat_factor = 0;
        break;
      }
      case 3: {
        ASSERT_EQ(transport::kError, result);
        repeat_factor = mock_rpcs::kFailureTolerance - 1;
        break;
      }
      case 4: {
        ASSERT_EQ(transport::kSuccess, result);
        break;
      }
      default:
        break;
    }
  }
}

TEST_F(MockRpcsTest, BEH_KAD_Rpcs_FindValue) {
  int repeat_factor(1);
  int result_type(1);
  for (int i = 0; i < 5; ++i) {
    boost::shared_ptr<MockRpcs> rpcs_(new MockRpcs(asio_service_, securifier_,
                                                   kFindValueRequest,
                                                   repeat_factor,
                                                   result_type));
    bool b(false), b2(false);
    int result(999);
    boost::mutex m;
    EXPECT_CALL(*rpcs_, Prepare(testing::_, testing::_))
        .WillOnce(testing::WithArgs<0, 1>(testing::Invoke(boost::bind(
        &MockRpcs::MockPrepare, rpcs_.get(), _1, _2))));

    Rpcs::FindValueFunctor fvf = boost::bind(&MockRpcsTest::FindValueCallback,
                                             this, _1, _2, _3, _4, _5, &b, &m,
                                             &result);

    boost::thread t1(&Rpcs::FindValue, rpcs_, NodeId(NodeId::kRandomId),
                     securifier_, peer_, fvf, kTcp);
    while (!b2) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      boost::mutex::scoped_lock loch_lomond(m);
      b2 = b;
    }
    switch (i) {
      case 0: {
        ASSERT_EQ(transport::kSuccess, result) << i;
        result_type = 2;
        break;
      }
      case 1: {
        ASSERT_EQ(transport::kError, result);
        result_type = 1;
        repeat_factor = mock_rpcs::kFailureTolerance;
        break;
      }
      case 2: {
        ASSERT_EQ(transport::kSuccess, result);
        repeat_factor = 0;
        break;
      }
      case 3: {
        ASSERT_EQ(transport::kError, result);
        repeat_factor = mock_rpcs::kFailureTolerance - 1;
        break;
      }
      case 4: {
        ASSERT_EQ(transport::kSuccess, result);
        break;
      }
      default:
        break;
    }
  }
}

TEST_F(MockRpcsTest, BEH_KAD_Rpcs_Downlist) {
  bool b(false), b2(false);
  int result(999);
  boost::mutex m;
  std::vector<NodeId> node_ids;
  NodeId node_id(NodeId::kRandomId);
  node_ids.push_back(node_id);
  boost::shared_ptr<MockRpcs> rpcs_(new MockRpcs(asio_service_, securifier_,
                                                 kDownlistNotification,
                                                 mock_rpcs::kFailureTolerance,
                                                 1));
  EXPECT_CALL(*rpcs_, Prepare(testing::_, testing::_))
      .WillOnce(testing::WithArgs<0, 1>(testing::Invoke(boost::bind(
      &MockRpcs::MockPrepare, rpcs_.get(), _1, _2))));

  boost::thread t1(&Rpcs::Downlist, rpcs_, node_ids, securifier_, peer_, kTcp);
  t1.join();
}

}   // namespace maidsafe

}   // namespace kademlia

}   // namespace test
