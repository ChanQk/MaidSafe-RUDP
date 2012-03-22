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

#include "maidsafe/transport/rudp_transport.h"

#include "boost/lexical_cast.hpp"
#include "boost/thread/condition_variable.hpp"
#include "boost/thread/mutex.hpp"

#include "maidsafe/common/asio_service.h"
#include "maidsafe/common/test.h"

#include "maidsafe/transport/log.h"


namespace maidsafe {

namespace transport {

namespace test {

namespace {

typedef std::vector<std::vector<std::string>> ReceivedMessages;
typedef std::vector<std::vector<std::pair<TransportCondition, Endpoint>>>
    ReceivedErrors;

void OnMessageReceived(const std::string &message,
                       const Info &/*info*/,
                       std::string *response,
                       Timeout *timeout,
                       boost::mutex *mutex,
                       const size_t &index,
                       ReceivedMessages *received_messages) {
  std::string index_str(boost::lexical_cast<std::string>(index));
  DLOG(INFO) << "Transport " << index << " received: " << message;
  response->assign("Transport " + index_str + " reply to \"" + message + "\"");
  *timeout = bptime::seconds(10);
  boost::mutex::scoped_lock lock(*mutex);
  received_messages->at(index).push_back(message);
}

void OnError(const TransportCondition &result,
             const Endpoint &endpoint,
             boost::mutex *mutex,
             const size_t &index,
             ReceivedErrors *received_errors) {
  std::string index_str(boost::lexical_cast<std::string>(index));
  DLOG(INFO) << "Transport " << index << " received error: " << result
             << " from " << endpoint.ip << ":" << endpoint.port;
  boost::mutex::scoped_lock lock(*mutex);
  received_errors->at(index).push_back(std::make_pair(result, endpoint));
}

void ConnectCallback(const TransportCondition &result_in,
                     TransportCondition *result_out,
                     boost::mutex *mutex,
                     boost::condition_variable *cond_var) {
  boost::mutex::scoped_lock lock(*mutex);
  *result_out = result_in;
  cond_var->notify_one();
}

}  // unnamed namespace


class RudpTransportTest : public testing::Test {
 protected:
  RudpTransportTest()
      : asio_services_(),
        rudp_transports_(),
        received_messages_(),
        received_errors_(),
        received_messages_mutex_(),
        received_errors_mutex_() {}

  void AddTransports(int count) {
    for (int i(0); i != count; ++i) {
      asio_services_.push_back(std::make_shared<AsioService>());
      (*asio_services_.rbegin())->Start(5);
      rudp_transports_.push_back(std::make_shared<RudpTransport>(
          (*asio_services_.rbegin())->service()));
      received_messages_.push_back(std::vector<std::string>());
      received_errors_.push_back(
          std::vector<std::pair<TransportCondition, Endpoint>>());
      (*rudp_transports_.rbegin())->on_message_received()->connect(std::bind(
          OnMessageReceived,
          args::_1, args::_2, args::_3, args::_4,
          &received_messages_mutex_,
          received_messages_.size() - 1,
          &received_messages_));
      (*rudp_transports_.rbegin())->on_error()->connect(std::bind(
          OnError,
          args::_1, args::_2,
          &received_errors_mutex_,
          received_errors_.size() - 1,
          &received_errors_));
    }
  }

  std::vector<std::shared_ptr<AsioService>> asio_services_;
  std::vector<std::shared_ptr<RudpTransport>> rudp_transports_;
  ReceivedMessages received_messages_;
  ReceivedErrors received_errors_;
  boost::mutex received_messages_mutex_, received_errors_mutex_;
};

TEST_F(RudpTransportTest, BEH_Connect) {
  AddTransports(2);
  Endpoint endpoint0("127.0.0.1",
                     static_cast<Port>(RandomUint32() % 25000) + 10000);
  Endpoint endpoint1("127.0.0.1",
                     static_cast<Port>(RandomUint32() % 25000) + 35000);
  ASSERT_EQ(kSuccess, rudp_transports_[0]->StartListening(endpoint0));
  ASSERT_EQ(kSuccess, rudp_transports_[1]->StartListening(endpoint1));
  TransportCondition result(kPendingResult);
  boost::mutex mutex;
  boost::condition_variable cond_var;
  rudp_transports_[0]->Connect(endpoint1, bptime::seconds(5),
      std::bind(ConnectCallback, args::_1, &result, &mutex, &cond_var));
  {
    boost::mutex::scoped_lock lock(mutex);
    EXPECT_TRUE(
        cond_var.timed_wait(lock,
                            bptime::seconds(6),
                            [&result]() { return result != kPendingResult; }));
  }
  EXPECT_EQ(kSuccess, result);
  Sleep(boost::posix_time::milliseconds(11000));
}


}  // namespace test

}  // namespace transport

}  // namespace maidsafe
