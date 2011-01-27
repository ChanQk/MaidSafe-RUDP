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

#ifndef MAIDSAFE_TESTS_DEMO_COMMANDS_H_
#define MAIDSAFE_TESTS_DEMO_COMMANDS_H_

#include <map>
#include <string>
#include "boost/function.hpp"
#include "maidsafe/common/platform_config.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace rpcprotocol {
class ChannelManager;
class NodeId;
typedef std::map<std::string, Stats<boost::uint64_t> > RpcStatsMap;
}  // namespace rpcprotocol

namespace kademlia {
class Node;
class NodeId;
}  // namespace kademlia

namespace kaddemo {

class Commands {
 public:
  Commands(boost::shared_ptr<kademlia::Node> node,
           boost::shared_ptr<rpcprotocol::ChannelManager> chmanager,
           const boost::uint16_t &K);
  void Run();
 private:
  void FindValueCallback(const std::string &result, const kademlia::NodeId &key,
                         const bool &write_to_file, const std::string &path);
  void StoreCallback(const std::string &result, const kademlia::NodeId &key,
                     const boost::int32_t &ttl);
  void PingCallback(const std::string &result, const kademlia::NodeId &id);
  void GetNodeContactDetailsCallback(const std::string &result,
                                     const kademlia::NodeId &id);
  void ProcessCommand(const std::string &cmdline, bool *wait_for_cb);
  void PrintUsage();
  bool ReadFile(const std::string &path, std::string *content);
  void WriteToFile(const std::string &path, const std::string &content);
  void Store50Values(const std::string &prefix);
  void Store50Callback(const std::string &result, const std::string &key,
                       bool *arrived);
  void PrintRpcTimings();
  boost::shared_ptr<kademlia::Node> node_;
  boost::shared_ptr<rpcprotocol::ChannelManager> chmanager_;
  bool result_arrived_, finish_;
  double min_succ_stores_;
  crypto::Crypto cryobj_;
};

}  // namespace kaddemo

}  // namespace maidsafe

#endif  // MAIDSAFE_TESTS_DEMO_COMMANDS_H_
