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
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <boost/thread/mutex.hpp>
#include <gtest/gtest.h>
#include <google/protobuf/descriptor.h>
#include <algorithm>
#include "maidsafe/base/log.h"
#include "maidsafe/base/calllatertimer.h"
#include "maidsafe/protobuf/rpcmessage.pb.h"
#include "maidsafe/protobuf/testservices.pb.h"
#include "maidsafe/rpcprotocol/channel-api.h"
#include "maidsafe/rpcprotocol/channelmanager-api.h"
#include "maidsafe/transport/transport.h"
#include "maidsafe/transport/udttransport.h"
#include "maidsafe/transport/tcptransport.h"

namespace test_rpcprotocol { size_t clients = 24; }

namespace rpcprotocol {

namespace test {

// NOTE: Put any new transport objects in here.

typedef testing::Types<transport::UdtTransport,
                       transport::TcpTransport> Implementations;
class PingTestService : public tests::PingTest {
 public:
  void TestPing(google::protobuf::RpcController*,
                const tests::TestPingRequest *request,
                tests::TestPingResponse *response,
                google::protobuf::Closure *done) {
//    DLOG(INFO) << "TestPing -- waiting for 1 second" << std::endl;
    boost::this_thread::sleep(boost::posix_time::seconds(1));
//    Controller *ctrler = static_cast<Controller*>(controller);
    if (request->IsInitialized()) {
      if (request->ping() == "ping") {
        response->set_result("S");
        response->set_pong("pong");
//        DLOG(INFO) << "Got ping request, returning response." << std::endl;
      } else {
        response->set_result("F");
        response->set_pong("");
//        DLOG(INFO) << "??????????? - " << request->ping() << " - "
//                   << request->ip() << " - " << request->port() << std::endl;
      }
    }
//    DLOG(INFO) << "PingRpc request!!!!!" << std::endl;
    done->Run();
  }
};

class TestOpService : public tests::TestOp {
 public:
  void Add(google::protobuf::RpcController*,
           const tests::BinaryOpRequest *request,
           tests::BinaryOpResponse *response,
           google::protobuf::Closure *done) {
    if (request->IsInitialized())
      response->set_result(request->first() + request->second());
//    Controller *ctrler = static_cast<Controller*>(controller);
//    DLOG(INFO) << "AddRpc request!!!!!" << std::endl;
    done->Run();
  }
  void Multiplyl(google::protobuf::RpcController*,
           const tests::BinaryOpRequest *request,
           tests::BinaryOpResponse *response,
           google::protobuf::Closure *done) {
    if (request->IsInitialized())
      response->set_result(request->first() * request->second());
//    Controller *ctrler = static_cast<Controller*>(controller);
//    DLOG(INFO) << "MultiplyRpc request rtt " << ctrler->rtt() << std::endl;
    done->Run();
  }
};

class MirrorTestService : public tests::MirrorTest {
 public:
  void Mirror(google::protobuf::RpcController*,
              const tests::StringMirrorRequest *request,
              tests::StringMirrorResponse *response,
              google::protobuf::Closure *done) {
    if (request->IsInitialized()) {
//      DLOG(INFO) << "Before reversing the string" << std::endl;
      std::string message(request->message());
      std::reverse(message.begin(), message.end());
      response->set_mirrored_string(message);
//      DLOG(INFO) << "Done reversing the string" << std::endl;
    } else {
      DLOG(INFO) << "Un-initialised request" << std::endl;
    }
//    Controller *ctrler = static_cast<Controller*>(controller);
//    DLOG(INFO) << "MirrorRpc request rtt " << ctrler->rtt() << std::endl;
    if (!request->has_not_pause() || !request->not_pause()) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(1500));
    }
    done->Run();
  }
};

class ResultHolder {
 public:
  ResultHolder() : ping_result_(),
                   op_result_(),
                   mirror_result_(),
                   result_arrived_(false),
                   mutex_(),
                   cond_var_() {
    op_result_.set_result(-1);
    mirror_result_.set_mirrored_string("-");
  }
  void HandlePingResponse(const tests::TestPingResponse *response,
                          const Controller *controller) {
    boost::mutex::scoped_lock lock(mutex_);
    if (controller->Failed() && controller->ErrorText() == kCancelled) {
      DLOG(INFO) << "Ping RPC canceled by the client." << std::endl;
      result_arrived_ = true;
      cond_var_.notify_one();
      return;
    }
    if (response->IsInitialized()) {
      ping_result_.set_result(response->result());
      ping_result_.set_pong(response->pong());
    } else {
      ping_result_.set_result("F");
    }
    result_arrived_ = true;
    cond_var_.notify_one();
  }
  void HandleOpResponse(const tests::BinaryOpResponse *response,
                        const Controller *controller) {
    boost::mutex::scoped_lock lock(mutex_);
    if (controller->Failed() && controller->ErrorText() == kCancelled) {
      DLOG(INFO) << "BinaryOperation RPC cancelled by the client" << std::endl;
      result_arrived_ = true;
      cond_var_.notify_one();
      return;
    }
    if (response->IsInitialized())
      op_result_.set_result(response->result());
    else
      op_result_.set_result(-2);
    result_arrived_ = true;
    cond_var_.notify_one();
  }
  void HandleMirrorResponse(const tests::StringMirrorResponse *response,
                            const Controller *controller) {
    boost::mutex::scoped_lock lock(mutex_);
    if (controller->Failed() && controller->ErrorText() == kCancelled) {
      DLOG(INFO) << "Mirror RPC cancelled by the client." << std::endl;
      result_arrived_ = true;
      cond_var_.notify_one();
      return;
    }
    if (response->IsInitialized()) {
      if (response->has_mirrored_string())
        mirror_result_.set_mirrored_string(response->mirrored_string());
      else
        mirror_result_.set_mirrored_string("+");
    } else {
      mirror_result_.set_mirrored_string("+");
    }
    result_arrived_ = true;
    cond_var_.notify_one();
  }
  void Reset() {
    boost::mutex::scoped_lock lock(mutex_);
    ping_result_.Clear();
    op_result_.Clear();
    op_result_.set_result(-1);
    mirror_result_.Clear();
    mirror_result_.set_mirrored_string("-");
    result_arrived_ = false;
  }
  tests::TestPingResponse ping_result() {
    boost::mutex::scoped_lock lock(mutex_);
    return ping_result_;
  }
  tests::BinaryOpResponse op_result() {
    boost::mutex::scoped_lock lock(mutex_);
    return op_result_;
  }
  tests::StringMirrorResponse mirror_result() {
    boost::mutex::scoped_lock lock(mutex_);
    return mirror_result_;
  }
  bool result_arrived() const { return result_arrived_; }
  void WaitForResponse(const boost::posix_time::milliseconds &timeout) {
    boost::mutex::scoped_lock lock(mutex_);
    try {
      bool wait_success = cond_var_.timed_wait(lock, timeout,
                          boost::bind(&ResultHolder::result_arrived, this));
      if (!wait_success) {
        DLOG(INFO) << "Failed to wait for result." << std::endl;
      }
    }
    catch(const std::exception &e) {
      DLOG(INFO) << "Error waiting for result: " << e.what() << std::endl;
    }
  }
 private:
  tests::TestPingResponse ping_result_;
  tests::BinaryOpResponse op_result_;
  tests::StringMirrorResponse mirror_result_;
  bool result_arrived_;
  boost::mutex mutex_;
  boost::condition_variable cond_var_;
};

typedef boost::shared_ptr<transport::Transport> TransportPtr;
typedef boost::shared_ptr<ChannelManager> ChannelManagerPtr;

template <class T>
class RpcProtocolTest : public testing::Test {
 public:
  RpcProtocolTest()
      : server_port_(0),
        server_channel_manager_(new ChannelManager(TransportPtr(new T))),
        client_channel_manager_(new ChannelManager(TransportPtr(new T))) {}
 protected:
  virtual void SetUp() {
    transport::TransportCondition tc;
    server_port_ = this->server_channel_manager_->transport()->
                   StartListening("127.0.0.1", 0, &tc);
    ASSERT_EQ(transport::kSuccess, tc);
//    server_channel_manager_.reset(new ChannelManager(
//        server_channel_manager_->transport()));
//    client_channel_manager_.reset(new ChannelManager(TransportPtr(new T)));
    ASSERT_EQ(0, server_channel_manager_->Start());
    ASSERT_EQ(0, client_channel_manager_->Start());
  }
  virtual void TearDown() {
    client_channel_manager_->Stop();
//    server_channel_manager_->transport()->StopListening(server_port_);
    server_channel_manager_->Stop();
  }
  Port server_port_;
  ChannelManagerPtr server_channel_manager_, client_channel_manager_;
};

TYPED_TEST_CASE(RpcProtocolTest, Implementations);

TYPED_TEST(RpcProtocolTest, BEH_RPC_RpcController) {
  Controller controller;
  ASSERT_FALSE(controller.Failed());
  ASSERT_TRUE(controller.ErrorText().empty());
  controller.SetFailed(kTimeOut);
//  SocketId id = 1234;
  ASSERT_TRUE(controller.Failed());
  ASSERT_EQ(kTimeOut, controller.ErrorText());
  controller.StartCancel();
  ASSERT_FALSE(controller.IsCanceled());
  controller.Reset();
  ASSERT_FALSE(controller.Failed());
  ASSERT_TRUE(controller.ErrorText().empty());
  ASSERT_EQ(0, controller.Duration());
  controller.StartRpcTimer();
  boost::this_thread::sleep(boost::posix_time::milliseconds(10));
  controller.StopRpcTimer();
  ASSERT_LE(10, controller.Duration());
  ASSERT_TRUE(controller.method().empty());
  controller.set_method("abc");
  ASSERT_EQ("abc", controller.method());
}

// TYPED_TEST(RpcProtocolTest, BEH_RPC_NoTransport) {
//   this->server_transport_->StopListening(this->server_port_);
//   PingTestService service;
//   creating a channel for the service
//   Channel service_channel(this->server_channel_manager_);
//   service_channel.SetService(&service);
//   this->server_channel_manager_->RegisterChannel(
//       service.GetDescriptor()->name(), &service_channel);
//   creating a channel for the client to send a request to the service
//   Controller controller;
//   controller.set_timeout(1000);
//   Channel out_channel(this->client_channel_manager_, "127.0.0.1",
//                       this->server_port_, "", 0);
//   tests::PingTest::Stub stubservice(&out_channel);
//   tests::TestPingRequest req;
//   tests::TestPingResponse resp;
//   req.set_ping("ping");
//   req.set_ip("127.0.0.1");
//   req.set_port(0);
//   ResultHolder resultholder;
//   google::protobuf::Closure *done =
//       google::protobuf::NewCallback<ResultHolder,
//                                     const tests::TestPingResponse*,
//                                     const Controller*>
//       (&resultholder, &ResultHolder::HandlePingResponse, &resp, &controller);
//   stubservice.TestPing(&controller, &req, &resp, done);
//   resultholder.WaitForResponse(boost::posix_time::milliseconds(10000));
//   ASSERT_EQ("F", resultholder.ping_result().result());
//   ASSERT_FALSE(resultholder.ping_result().has_pong());
// }
/*
TYPED_TEST(RpcProtocolTest, BEH_RPC_NoService) {
  // creating a channel for the client to send a request to the non-service
  Controller controller;
  controller.set_timeout(1000);
  Channel out_channel(this->client_channel_manager_, "127.0.0.1",
                      this->server_port_, "", 0);
  tests::PingTest::Stub stubservice(&out_channel);
  tests::TestPingRequest req;
  tests::TestPingResponse resp;
  req.set_ping("ping");
  req.set_ip("127.0.0.1");
  req.set_port(0);
  ResultHolder resultholder;
  google::protobuf::Closure *done =
      google::protobuf::NewCallback<ResultHolder,
                                    const tests::TestPingResponse*,
                                    const Controller*>
      (&resultholder, &ResultHolder::HandlePingResponse, &resp, &controller);
  stubservice.TestPing(&controller, &req, &resp, done);
  resultholder.WaitForResponse(boost::posix_time::milliseconds(5000));
  ASSERT_EQ("F", resultholder.ping_result().result());
  ASSERT_FALSE(resultholder.ping_result().has_pong());
}*/

TYPED_TEST(RpcProtocolTest, BEH_RPC_RegisterAChannel) {
  PingTestService service;
  // creating a channel for the service
  Channel service_channel(this->server_channel_manager_);
  service_channel.SetService(&service);
  this->server_channel_manager_->RegisterChannel(
      service.GetDescriptor()->name(), &service_channel);
  // creating a channel for the client to send a request to the service
  Controller controller;
  controller.set_timeout(5000);
  Channel out_channel(this->client_channel_manager_, "127.0.0.1",
                      this->server_port_, "", 0);
  tests::PingTest::Stub stubservice(&out_channel);
  tests::TestPingRequest req;
  tests::TestPingResponse resp;
  req.set_ping("ping");
  req.set_ip("127.0.0.1");
  req.set_port(0);
  ResultHolder resultholder;
  google::protobuf::Closure *done =
      google::protobuf::NewCallback<ResultHolder,
                                    const tests::TestPingResponse*,
                                    const Controller*>
      (&resultholder, &ResultHolder::HandlePingResponse, &resp, &controller);
  stubservice.TestPing(&controller, &req, &resp, done);
  resultholder.WaitForResponse(boost::posix_time::milliseconds(10000));

  ASSERT_TRUE(resultholder.ping_result().result());
  ASSERT_TRUE(resultholder.ping_result().has_pong());
  ASSERT_EQ("pong", resultholder.ping_result().pong());
}

TYPED_TEST(RpcProtocolTest, BEH_RPC_MultipleChannelsRegistered) {
  PingTestService service1;
  TestOpService service2;
  MirrorTestService service3;
  MirrorTestService service4;

  // creating a channel for the service
  Channel service_channel1(this->server_channel_manager_);
  service_channel1.SetService(&service1);
  this->server_channel_manager_->RegisterChannel(
      service1.GetDescriptor()->name(), &service_channel1);

  Channel service_channel2(this->server_channel_manager_);
  service_channel2.SetService(&service2);
  this->server_channel_manager_->RegisterChannel(
      service2.GetDescriptor()->name(), &service_channel2);

  Channel service_channel3(this->server_channel_manager_);
  service_channel3.SetService(&service3);
  this->server_channel_manager_->RegisterChannel(
      service3.GetDescriptor()->name(), &service_channel3);

  Channel service_channel4(this->server_channel_manager_);
  service_channel4.SetService(&service4);
  this->server_channel_manager_->RegisterChannel(
      service4.GetDescriptor()->name(), &service_channel4);

  // creating a channel for the client to send a request to the service
  Controller controller;
  controller.set_timeout(5000);
  Channel out_channel1(this->client_channel_manager_, "127.0.0.1",
                       this->server_port_, "", 0);
  tests::PingTest::Stub stubservice1(&out_channel1);
  tests::TestPingRequest req1;
  tests::TestPingResponse resp1;
  req1.set_ping("ping");
  req1.set_ip("127.0.0.1");
  req1.set_port(0);
  ResultHolder resultholder;
  google::protobuf::Closure *done1 =
      google::protobuf::NewCallback<ResultHolder,
                                    const tests::TestPingResponse*,
                                    const Controller*>
      (&resultholder, &ResultHolder::HandlePingResponse, &resp1, &controller);
  stubservice1.TestPing(&controller, &req1, &resp1, done1);
  resultholder.WaitForResponse(boost::posix_time::milliseconds(10000));
  ASSERT_TRUE(resultholder.ping_result().result());
  ASSERT_TRUE(resultholder.ping_result().has_pong());
  ASSERT_EQ("pong", resultholder.ping_result().pong());
  resultholder.Reset();

  Channel out_channel2(this->client_channel_manager_, "127.0.0.1",
                       this->server_port_, "", 0);
  tests::TestOp::Stub stubservice2(&out_channel2);
  tests::BinaryOpRequest req2;
  tests::BinaryOpResponse resp2;
  req2.set_first(3);
  req2.set_second(2);
  req2.set_ip("127.0.0.1");
  req2.set_port(0);
  Controller controller2;
  google::protobuf::Closure *done2 =
      google::protobuf::NewCallback<ResultHolder,
                                    const tests::BinaryOpResponse*,
                                    const Controller*>
      (&resultholder, &ResultHolder::HandleOpResponse, &resp2, &controller2);
  controller2.set_timeout(6000);
  stubservice2.Add(&controller2, &req2, &resp2, done2);
  resultholder.WaitForResponse(boost::posix_time::milliseconds(12000));
  ASSERT_EQ(5, resultholder.op_result().result());
  resultholder.Reset();

  std::string test_string;
  test_string.reserve(5 * 1024 * 1024);
  std::string random_substring(base::RandomString(1024));
  for (int i = 0; i < 5 * 1024; ++i)
    test_string += random_substring;
  Channel out_channel3(this->client_channel_manager_, "127.0.0.1",
                       this->server_port_, "", 0);
  tests::MirrorTest::Stub stubservice3(&out_channel3);
  tests::StringMirrorRequest req3;
  tests::StringMirrorResponse resp3;
  req3.set_message(test_string);
  req3.set_ip("127.0.0.1");
  req3.set_port(0);
  Controller controller3;
  google::protobuf::Closure *done3 =
      google::protobuf::NewCallback<ResultHolder,
                                    const tests::StringMirrorResponse*,
                                    const Controller*>
      (&resultholder, &ResultHolder::HandleMirrorResponse, &resp3,
       &controller3);
  controller3.set_timeout(1000);
  stubservice3.Mirror(&controller3, &req3, &resp3, done3);
  resultholder.WaitForResponse(boost::posix_time::milliseconds(20000));
  if ("+" != resultholder.mirror_result().mirrored_string()) {
    this->server_channel_manager_->ClearCallLaters();
    this->client_channel_manager_->ClearCallLaters();
    FAIL() << "Operation did not time out.";
  }
  resultholder.Reset();

  Channel out_channel4(this->client_channel_manager_, "127.0.0.1",
                       this->server_port_, "", 0);
  tests::MirrorTest::Stub stubservice4(&out_channel4);
  tests::StringMirrorRequest req4;
  tests::StringMirrorResponse resp4;
  test_string.replace(test_string.size()-10, 10, "0123456789");
  req4.set_message(test_string);
  req4.set_ip("127.0.0.1");
  req4.set_port(0);
  Controller controller4;
  google::protobuf::Closure *done4 =
      google::protobuf::NewCallback<ResultHolder,
                                    const tests::StringMirrorResponse*,
                                    const Controller*>
      (&resultholder, &ResultHolder::HandleMirrorResponse, &resp4,
       &controller4);
  controller4.set_timeout(20000);
  stubservice4.Mirror(&controller4, &req4, &resp4, done4);

  resultholder.WaitForResponse(boost::posix_time::milliseconds(40000));
  resultholder.WaitForResponse(boost::posix_time::milliseconds(140000));
  // FIXME - invalid template arguments
//   if ("+" == resultholder.mirror_result().mirrored_string()) {
//     this->RpcProtocolTest::server_channel_manager_->ClearCallLaters();
//     this->RpcProtocolTest::client_channel_manager_->ClearCallLaters();
//     FAIL() << "Result of mirror RPC is incorrect.";
//   }
//   ASSERT_EQ("9876543210",
//             resultholder.mirror_result().mirrored_string().substr(0, 10));
}

TYPED_TEST(RpcProtocolTest, BEH_RPC_ServerAndClientCommunication) {
  PingTestService service1;
  Channel service_channel1(this->server_channel_manager_);
  service_channel1.SetService(&service1);
  this->server_channel_manager_->RegisterChannel(
      service1.GetDescriptor()->name(), &service_channel1);

  boost::shared_ptr<transport::UdtTransport> some_udt_transport(
      new transport::UdtTransport);
  transport::TransportCondition tc;
  Port p = some_udt_transport->StartListening("127.0.0.1", 0, &tc);
  ASSERT_EQ(transport::kSuccess, tc);
  ChannelManagerPtr some_chann_manager(new ChannelManager(some_udt_transport));
  ASSERT_EQ(0, some_chann_manager->Start());
  TestOpService service2;
  Channel service_channel2(some_chann_manager);
  service_channel2.SetService(&service2);
  some_chann_manager->RegisterChannel(service2.GetDescriptor()->name(),
                                      &service_channel2);

  Controller controller1;
  controller1.set_timeout(5000);
  Channel out_channel1(this->server_channel_manager_, "127.0.0.1", p, "", 0);

  tests::TestOp::Stub stubservice1(&out_channel1);
  tests::BinaryOpRequest req1;
  tests::BinaryOpResponse resp1;
  req1.set_first(3);
  req1.set_second(2);
  req1.set_ip("127.0.0.1");
  req1.set_port(0);

  ResultHolder resultholder;
  google::protobuf::Closure *done1 =
      google::protobuf::NewCallback<ResultHolder,
                                    const tests::BinaryOpResponse*,
                                    const Controller*>
      (&resultholder, &ResultHolder::HandleOpResponse, &resp1, &controller1);
  stubservice1.Add(&controller1, &req1, &resp1, done1);
  resultholder.WaitForResponse(boost::posix_time::milliseconds(10000));
  ASSERT_EQ(5, resultholder.op_result().result());

  Controller controller;
  controller.set_timeout(5000);
  Channel out_channel(this->client_channel_manager_, "127.0.0.1",
                      this->server_port_, "", 0);
  tests::PingTest::Stub stubservice(&out_channel);
  tests::TestPingRequest req;
  tests::TestPingResponse resp;
  req.set_ping("ping");
  req.set_ip("127.0.0.1");
  req.set_port(0);
  resultholder.Reset();
  google::protobuf::Closure *done =
      google::protobuf::NewCallback<ResultHolder,
                                    const tests::TestPingResponse*,
                                    const Controller*>
      (&resultholder, &ResultHolder::HandlePingResponse, &resp, &controller);
  stubservice.TestPing(&controller, &req, &resp, done);
  resultholder.WaitForResponse(boost::posix_time::milliseconds(10000));

  ASSERT_TRUE(resultholder.ping_result().result());
  ASSERT_TRUE(resultholder.ping_result().has_pong());
  ASSERT_EQ("pong", resultholder.ping_result().pong());

  some_udt_transport->StopListening(p);
  some_chann_manager->Stop();
}

TYPED_TEST(RpcProtocolTest, BEH_RPC_TriggerPendingRequest) {
  MirrorTestService mirrorservice;
  Channel service_channel(this->server_channel_manager_);
  service_channel.SetService(&mirrorservice);
  this->server_channel_manager_->RegisterChannel(
      mirrorservice.GetDescriptor()->name(), &service_channel);
  // Sending rpc to an existing server, but deleting it before response arrives
  Controller controller;
  controller.set_timeout(10000);
  Channel out_channel1(this->client_channel_manager_, "127.0.0.1",
                       this->server_port_, "", 0);
  std::string test_string(base::RandomString(500 * 1024));
  tests::MirrorTest::Stub stubservice1(&out_channel1);
  tests::StringMirrorRequest req1, req2;
  tests::StringMirrorResponse resp1, resp2;
  req1.set_message(test_string);
  req1.set_ip("127.0.0.1");
  req1.set_port(0);
  ResultHolder resultholder;
  google::protobuf::Closure *done1 =
      google::protobuf::NewCallback<ResultHolder,
                                    const tests::StringMirrorResponse*,
                                    const Controller*>
      (&resultholder, &ResultHolder::HandleMirrorResponse, &resp1,
       &controller);
  stubservice1.Mirror(&controller, &req1, &resp1, done1);

  ASSERT_TRUE(this->client_channel_manager_->TriggerPendingRequest(
              controller.socket_id()));
  boost::this_thread::sleep(boost::posix_time::seconds(11));
  ASSERT_EQ(std::string("-"), resultholder.mirror_result().mirrored_string());
  ASSERT_EQ(kCancelled, controller.ErrorText());
  controller.Reset();
  ASSERT_TRUE(controller.ErrorText().empty());
}

TYPED_TEST(RpcProtocolTest, BEH_RPC_DeletePendingRequest) {
  MirrorTestService mirrorservice;
  Channel service_channel(this->server_channel_manager_);
  service_channel.SetService(&mirrorservice);
  this->server_channel_manager_->RegisterChannel(
      mirrorservice.GetDescriptor()->name(), &service_channel);
  // Send RPC to an existing server, but deleting it before response arrives
  Controller controller;
  controller.set_timeout(10000);
  Channel out_channel1(this->client_channel_manager_, "127.0.0.1",
                       this->server_port_, "", 0);
  std::string test_string(base::RandomString(500 * 1024));
  tests::MirrorTest::Stub stubservice1(&out_channel1);
  tests::StringMirrorRequest req1, req2;
  tests::StringMirrorResponse resp1, resp2;
  req1.set_message(test_string);
  req1.set_ip("127.0.0.1");
  req1.set_port(0);
  ResultHolder resultholder;
  google::protobuf::Closure *done1 =
      google::protobuf::NewCallback<ResultHolder,
                                    const tests::StringMirrorResponse*,
                                    const Controller*>
      (&resultholder, &ResultHolder::HandleMirrorResponse, &resp1, &controller);
  stubservice1.Mirror(&controller, &req1, &resp1, done1);
  ASSERT_TRUE(this->client_channel_manager_->DeletePendingRequest(
              controller.socket_id()));
  ASSERT_FALSE(this->client_channel_manager_->DeletePendingRequest(
               controller.socket_id()));
  controller.Reset();
  ASSERT_EQ(std::string(""), controller.ErrorText());
}
/**/
void SendPingsThread(
    Port server_port,
    boost::shared_ptr< std::vector<ChannelManagerPtr> > p_clients,
    std::vector<bool> *res_pings) {
  for (size_t n = 0; n < p_clients->size(); ++n) {
    Controller controller;
    controller.set_timeout(5000);
    Channel out_channel(p_clients->at(n), "127.0.0.1", server_port, "", 0);
    tests::PingTest::Stub stubservice(&out_channel);
    tests::TestPingRequest req;
    tests::TestPingResponse resp;
    req.set_ping("ping");
    req.set_ip("127.0.0.1");
    req.set_port(0);
    ResultHolder resultholder;
    google::protobuf::Closure *done =
        google::protobuf::NewCallback<ResultHolder,
                                      const tests::TestPingResponse*,
                                      const Controller*>
        (&resultholder, &ResultHolder::HandlePingResponse, &resp, &controller);
    stubservice.TestPing(&controller, &req, &resp, done);
    resultholder.WaitForResponse(boost::posix_time::milliseconds(10000));

    if (resultholder.ping_result().result() &&
        resultholder.ping_result().has_pong() &&
        "pong" == resultholder.ping_result().pong())
      res_pings->at(n) = true;
  }
}

void SendOpsThread(
    Port server_port,
    boost::shared_ptr< std::vector<ChannelManagerPtr> > o_clients,
    std::vector<bool> *res_ops) {
  for (size_t n = 0; n < o_clients->size(); ++n) {
    Controller controller;
    controller.set_timeout(5000);
    Channel out_channel(o_clients->at(n), "127.0.0.1", server_port, "", 0);
    tests::TestOp::Stub stubservice2(&out_channel);
    tests::BinaryOpRequest req2;
    tests::BinaryOpResponse resp2;
    req2.set_first(3);
    req2.set_second(2);
    req2.set_ip("127.0.0.1");
    req2.set_port(0);
    ResultHolder resultholder;
    google::protobuf::Closure *done2 =
        google::protobuf::NewCallback<ResultHolder,
                                      const tests::BinaryOpResponse*,
                                      const Controller*>
        (&resultholder, &ResultHolder::HandleOpResponse, &resp2, &controller);
    stubservice2.Add(&controller, &req2, &resp2, done2);
    resultholder.WaitForResponse(boost::posix_time::milliseconds(10000));
    if (5 == resultholder.op_result().result())
      res_ops->at(n) = true;
  }
}

void SendMirrorsThread(
    Port server_port,
    boost::shared_ptr< std::vector<ChannelManagerPtr> > m_clients,
    std::vector<bool> *res_mirrors) {
  for (size_t n = 0; n < m_clients->size(); ++n) {
    Controller controller;
    controller.set_timeout(10000);
    Channel out_channel(m_clients->at(n), "127.0.0.1", server_port, "", 0);
    tests::MirrorTest::Stub stubservice4(&out_channel);
    tests::StringMirrorRequest req4;
    tests::StringMirrorResponse resp4;
    std::string test_string("0123456789");
    req4.set_message(test_string);
    req4.set_ip("127.0.0.1");
    req4.set_port(0);
    ResultHolder resultholder;
    google::protobuf::Closure *done4 =
        google::protobuf::NewCallback<ResultHolder,
                                      const tests::StringMirrorResponse*,
                                      const Controller*>
        (&resultholder, &ResultHolder::HandleMirrorResponse, &resp4,
         &controller);
    stubservice4.Mirror(&controller, &req4, &resp4, done4);

    resultholder.WaitForResponse(boost::posix_time::milliseconds(20000));

    if ("9876543210" ==
        resultholder.mirror_result().mirrored_string().substr(0, 10))
      res_mirrors->at(n) = true;
  }
}

TYPED_TEST(RpcProtocolTest, FUNC_RPC_ThreadedClientsOneServer) {
  PingTestService service1;
  TestOpService service2;
  MirrorTestService service3;

  // creating a channel for the service
  Channel service_channel1(this->server_channel_manager_);
  service_channel1.SetService(&service1);
  this->server_channel_manager_->RegisterChannel(
      service1.GetDescriptor()->name(), &service_channel1);

  Channel service_channel2(this->server_channel_manager_);
  service_channel2.SetService(&service2);
  this->server_channel_manager_->RegisterChannel(
      service2.GetDescriptor()->name(), &service_channel2);

  Channel service_channel3(this->server_channel_manager_);
  service_channel3.SetService(&service3);
  this->server_channel_manager_->RegisterChannel(
      service3.GetDescriptor()->name(), &service_channel3);

  boost::shared_ptr< std::vector<ChannelManagerPtr> >
      ping_clients(new std::vector<ChannelManagerPtr>);
  boost::shared_ptr< std::vector<ChannelManagerPtr> >
      op_clients(new std::vector<ChannelManagerPtr>);
  boost::shared_ptr< std::vector<ChannelManagerPtr> >
      mirror_clients(new std::vector<ChannelManagerPtr>);

  std::vector<bool> res_pings(test_rpcprotocol::clients, false);
  std::vector<bool> res_ops(test_rpcprotocol::clients, false);
  std::vector<bool> res_mirrors(test_rpcprotocol::clients, false);

  ping_clients->resize(test_rpcprotocol::clients);
  op_clients->resize(test_rpcprotocol::clients);
  mirror_clients->resize(test_rpcprotocol::clients);
  for (size_t n = 0; n < test_rpcprotocol::clients; ++n) {
    ping_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    ping_clients->at(n)->Start();
    op_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    op_clients->at(n)->Start();
    mirror_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    mirror_clients->at(n)->Start();
  }

  boost::thread th_pings(&SendPingsThread, this->server_port_, ping_clients,
                         &res_pings);
  boost::thread th_ops(&SendOpsThread, this->server_port_,
                       op_clients, &res_ops);
  boost::thread th_mirrors(&SendMirrorsThread, this->server_port_,
                           mirror_clients, &res_mirrors);
  th_pings.join();
  th_ops.join();
  th_mirrors.join();

  bool success(true);
  for (size_t a = 0; a < test_rpcprotocol::clients; ++a) {
    if (!res_pings[a] || !res_ops[a] || !res_mirrors[a]) {
      success = false;
      a = test_rpcprotocol::clients;
    }
  }

  for (size_t y = 0; y < test_rpcprotocol::clients; ++y) {
    ping_clients->at(y)->Stop();
    op_clients->at(y)->Stop();
    mirror_clients->at(y)->Stop();
  }
  ASSERT_TRUE(success);
}

TYPED_TEST(RpcProtocolTest, FUNC_RPC_ThreadedClientsManyServers) {
  // ******************************** Server A ****************************
  PingTestService serviceA1;
  TestOpService serviceA2;
  MirrorTestService serviceA3;
  Channel service_channelA1(this->server_channel_manager_);
  service_channelA1.SetService(&serviceA1);
  this->server_channel_manager_->RegisterChannel(
      serviceA1.GetDescriptor()->name(), &service_channelA1);
  Channel service_channelA2(this->server_channel_manager_);
  service_channelA2.SetService(&serviceA2);
  this->server_channel_manager_->RegisterChannel(
      serviceA2.GetDescriptor()->name(), &service_channelA2);
  Channel service_channelA3(this->server_channel_manager_);
  service_channelA3.SetService(&serviceA3);
  this->server_channel_manager_->RegisterChannel(
      serviceA3.GetDescriptor()->name(), &service_channelA3);

  // ******************************** Server B ********************************
  boost::shared_ptr<transport::UdtTransport> Bserver_transport;
  boost::shared_ptr<ChannelManager> Bserver_chann_manager;
  Bserver_transport.reset(new transport::UdtTransport);
  transport::TransportCondition Btc;
  Port Bserver_port = Bserver_transport->StartListening("127.0.0.1", 0, &Btc);
  ASSERT_EQ(transport::kSuccess, Btc);
  Bserver_chann_manager.reset(new ChannelManager(Bserver_transport));
  ASSERT_EQ(0, Bserver_chann_manager->Start());

  PingTestService serviceB1;
  TestOpService serviceB2;
  MirrorTestService serviceB3;
  Channel service_channelB1(Bserver_chann_manager);
  service_channelB1.SetService(&serviceB1);
  Bserver_chann_manager->RegisterChannel(serviceB1.GetDescriptor()->name(),
                                         &service_channelB1);
  Channel service_channelB2(Bserver_chann_manager);
  service_channelB2.SetService(&serviceB2);
  Bserver_chann_manager->RegisterChannel(serviceB2.GetDescriptor()->name(),
                                         &service_channelB2);
  Channel service_channelB3(Bserver_chann_manager);
  service_channelB3.SetService(&serviceB3);
  Bserver_chann_manager->RegisterChannel(serviceB3.GetDescriptor()->name(),
                                         &service_channelB3);

  // ******************************* Server C **********************************
  boost::shared_ptr<transport::UdtTransport> Cserver_transport;
  boost::shared_ptr<ChannelManager> Cserver_chann_manager;
  Cserver_transport.reset(new transport::UdtTransport);
  transport::TransportCondition Ctc;
  Port Cserver_port = Cserver_transport->StartListening("127.0.0.1", 0, &Ctc);
  ASSERT_EQ(transport::kSuccess, Ctc);
  Cserver_chann_manager.reset(new ChannelManager(Cserver_transport));
  ASSERT_EQ(0, Cserver_chann_manager->Start());

  PingTestService serviceC1;
  TestOpService serviceC2;
  MirrorTestService serviceC3;
  Channel service_channelC1(Cserver_chann_manager);
  service_channelC1.SetService(&serviceC1);
  Cserver_chann_manager->RegisterChannel(serviceC1.GetDescriptor()->name(),
                                         &service_channelC1);
  Channel service_channelC2(Cserver_chann_manager);
  service_channelC2.SetService(&serviceC2);
  Cserver_chann_manager->RegisterChannel(serviceC2.GetDescriptor()->name(),
                                         &service_channelC2);
  Channel service_channelC3(Cserver_chann_manager);
  service_channelC3.SetService(&serviceC3);
  Cserver_chann_manager->RegisterChannel(serviceC3.GetDescriptor()->name(),
                                         &service_channelC3);

  // ******************************** Clients A ********************************
  boost::shared_ptr<std::vector<ChannelManagerPtr> >
      Aping_clients(new std::vector<ChannelManagerPtr>);
  boost::shared_ptr< std::vector<ChannelManagerPtr> >
      Aop_clients(new std::vector<ChannelManagerPtr>);
  boost::shared_ptr< std::vector<ChannelManagerPtr> >
      Amirror_clients(new std::vector<ChannelManagerPtr>);

  std::vector<bool> Ares_pings(test_rpcprotocol::clients, false);
  std::vector<bool> Ares_ops(test_rpcprotocol::clients, false);
  std::vector<bool> Ares_mirrors(test_rpcprotocol::clients, false);

  Aping_clients->resize(test_rpcprotocol::clients);
  Aop_clients->resize(test_rpcprotocol::clients);
  Amirror_clients->resize(test_rpcprotocol::clients);
  for (size_t n = 0; n < test_rpcprotocol::clients; ++n) {
    Aping_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    Aping_clients->at(n)->Start();
    Aop_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    Aop_clients->at(n)->Start();
    Amirror_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    Amirror_clients->at(n)->Start();
  }

  // ******************************** Clients B ********************************
  boost::shared_ptr< std::vector<ChannelManagerPtr> >
      Bping_clients(new std::vector<ChannelManagerPtr>);
  boost::shared_ptr< std::vector<ChannelManagerPtr> >
      Bop_clients(new std::vector<ChannelManagerPtr>);
  boost::shared_ptr< std::vector<ChannelManagerPtr> >
      Bmirror_clients(new std::vector<ChannelManagerPtr>);

  std::vector<bool> Bres_pings(test_rpcprotocol::clients, false);
  std::vector<bool> Bres_ops(test_rpcprotocol::clients, false);
  std::vector<bool> Bres_mirrors(test_rpcprotocol::clients, false);

  Bping_clients->resize(test_rpcprotocol::clients);
  Bop_clients->resize(test_rpcprotocol::clients);
  Bmirror_clients->resize(test_rpcprotocol::clients);
  for (size_t n = 0; n < test_rpcprotocol::clients; ++n) {
    Bping_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    Bping_clients->at(n)->Start();
    Bop_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    Bop_clients->at(n)->Start();
    Bmirror_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    Bmirror_clients->at(n)->Start();
  }

  // ******************************* Clients C *********************************
  boost::shared_ptr< std::vector<ChannelManagerPtr> >
      Cping_clients(new std::vector<ChannelManagerPtr>);
  boost::shared_ptr<std::vector<ChannelManagerPtr> >
      Cop_clients(new std::vector<ChannelManagerPtr>);
  boost::shared_ptr<std::vector<ChannelManagerPtr> >
      Cmirror_clients(new std::vector<ChannelManagerPtr>);

  std::vector<bool> Cres_pings(test_rpcprotocol::clients, false);
  std::vector<bool> Cres_ops(test_rpcprotocol::clients, false);
  std::vector<bool> Cres_mirrors(test_rpcprotocol::clients, false);

  Cping_clients->resize(test_rpcprotocol::clients);
  Cop_clients->resize(test_rpcprotocol::clients);
  Cmirror_clients->resize(test_rpcprotocol::clients);
  for (size_t n = 0; n < test_rpcprotocol::clients; ++n) {
    Cping_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    Cping_clients->at(n)->Start();
    Cop_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    Cop_clients->at(n)->Start();
    Cmirror_clients->at(n).reset(
        new ChannelManager(boost::shared_ptr<transport::Transport>()));
    Cmirror_clients->at(n)->Start();
  }

  // ******************************** Start threads ****************************
  boost::thread th_pingsA(&SendPingsThread, this->server_port_, Aping_clients,
                          &Ares_pings);
  boost::thread th_opsA(&SendOpsThread, Bserver_port, Aop_clients, &Ares_ops);
  boost::thread th_mirrorsA(&SendMirrorsThread, Cserver_port, Amirror_clients,
                            &Ares_mirrors);
  boost::thread th_pingsB(&SendPingsThread, Bserver_port, Bping_clients,
                          &Bres_pings);
  boost::thread th_opsB(&SendOpsThread, Cserver_port, Bop_clients, &Bres_ops);
  boost::thread th_mirrorsB(&SendMirrorsThread, this->server_port_,
                            Bmirror_clients, &Bres_mirrors);
  boost::thread th_pingsC(&SendPingsThread, Cserver_port, Cping_clients,
                          &Cres_pings);
  boost::thread th_opsC(&SendOpsThread, this->server_port_, Cop_clients,
                        &Cres_ops);
  boost::thread th_mirrorsC(&SendMirrorsThread, Bserver_port, Cmirror_clients,
                            &Cres_mirrors);

  th_pingsA.join();
  th_opsA.join();
  th_mirrorsA.join();
  th_pingsB.join();
  th_opsB.join();
  th_mirrorsB.join();
  th_pingsC.join();
  th_opsC.join();
  th_mirrorsC.join();

  bool success(true);
  for (size_t a = 0; a < test_rpcprotocol::clients; ++a) {
    if (!Ares_pings[a] || !Ares_ops[a] || !Ares_mirrors[a] ||
        !Bres_pings[a] || !Bres_ops[a] || !Bres_mirrors[a] ||
        !Cres_pings[a] || !Cres_ops[a] || !Cres_mirrors[a]) {
      success = false;
      a = test_rpcprotocol::clients;
    }
  }

  for (size_t y = 0; y < test_rpcprotocol::clients; ++y) {
    Aping_clients->at(y)->Stop();
    Aop_clients->at(y)->Stop();
    Amirror_clients->at(y)->Stop();
    Bping_clients->at(y)->Stop();
    Bop_clients->at(y)->Stop();
    Bmirror_clients->at(y)->Stop();
    Cping_clients->at(y)->Stop();
    Cop_clients->at(y)->Stop();
    Cmirror_clients->at(y)->Stop();
  }

  Bserver_transport->StopListening(this->server_port_);
  Bserver_chann_manager->Stop();
  Cserver_transport->StopListening(this->server_port_);
  Cserver_chann_manager->Stop();

  ASSERT_TRUE(success);
}

}  // namespace test

}  // namespace rpcprotocol
