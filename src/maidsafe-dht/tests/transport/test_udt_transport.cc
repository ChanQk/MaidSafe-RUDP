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

#include "boost/thread.hpp"
#include "gtest/gtest.h"
#include "maidsafe-dht/common/log.h"
#include "maidsafe-dht/transport/udt_transport.h"
#include "maidsafe-dht/tests/transport/test_transport_api.h"

namespace maidsafe {

namespace transport {

namespace test {

TEST(UdtTransportTest, BEH_MAID_Transport) {
  std::shared_ptr<boost::asio::io_service> asio_service1(
      new boost::asio::io_service);
  std::shared_ptr<boost::asio::io_service> asio_service2(
      new boost::asio::io_service);
  std::shared_ptr<boost::asio::io_service::work>
      work1(new boost::asio::io_service::work(*asio_service1));
  std::shared_ptr<boost::asio::io_service::work>
      work2(new boost::asio::io_service::work(*asio_service2));
  boost::thread_group threads1, threads2;
  threads1.create_thread(std::bind(static_cast<
        std::size_t(boost::asio::io_service::*)()>
            (&boost::asio::io_service::run), asio_service1));
  threads1.create_thread(std::bind(static_cast<
        std::size_t(boost::asio::io_service::*)()>
            (&boost::asio::io_service::run), asio_service1));
  threads1.create_thread(std::bind(static_cast<
        std::size_t(boost::asio::io_service::*)()>
            (&boost::asio::io_service::run), asio_service1));
  threads2.create_thread(std::bind(static_cast<
        std::size_t(boost::asio::io_service::*)()>
            (&boost::asio::io_service::run), asio_service2));
  threads2.create_thread(std::bind(static_cast<
        std::size_t(boost::asio::io_service::*)()>
            (&boost::asio::io_service::run), asio_service2));
  std::shared_ptr<UdtTransport> transport1(new UdtTransport(asio_service1));
  TestMessageHandler message_handler1("message_handler1");
  TestMessageHandler message_handler2("message_handler2");
  transport1->on_message_received()->connect(boost::bind(
      &TestMessageHandler::DoOnRequestReceived, &message_handler1, _1, _2, _3,
      _4));
  transport1->on_error()->connect(boost::bind(&TestMessageHandler::DoOnError,
                                              &message_handler1, _1));
  Endpoint listening_endpoint("127.0.0.1", 9000);
  EXPECT_EQ(kSuccess, transport1->StartListening(listening_endpoint));
  for (int i = 0; i < 200; ++i) {
    std::shared_ptr<UdtTransport> transport2(new UdtTransport(asio_service2));
    transport2->on_message_received()->connect(boost::bind(
        &TestMessageHandler::DoOnResponseReceived, &message_handler2, _1, _2, _3,
        _4));
    transport2->on_error()->connect(boost::bind(&TestMessageHandler::DoOnError,
                                                &message_handler2, _1));
    transport2->Send("Test", listening_endpoint, boost::posix_time::seconds(1));
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  }
  transport1->StopListening();
  work1.reset();
  work2.reset();
  threads1.join_all();
  threads2.join_all();
}

}  // namespace test

}  // namespace transport

}  // namespace maidsafe
