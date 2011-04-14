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

#ifndef MAIDSAFE_DHT_TESTS_DEMO_COMMANDS_H_
#define MAIDSAFE_DHT_TESTS_DEMO_COMMANDS_H_

#include <string>
#include<vector>

#include "boost/date_time/posix_time/posix_time_types.hpp"

#include "maidsafe/common/platform_config.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/dht/kademlia/config.h"
#include "maidsafe/dht/kademlia/securifier.h"

namespace bptime = boost::posix_time;

namespace maidsafe {

namespace dht {

namespace kademlia {

class NodeId;
class Node;

namespace kaddemo {

class Commands {
 public:
  Commands(std::shared_ptr<Node> node,
           std::shared_ptr<Securifier> securifier,
           const boost::uint16_t &K);
  void Run();
 private:
  void print_node_info(const Contact &contact);

  void Store(const std::vector<std::string> &args, bool *wait_for_cb,
             bool read_from_file);

  void FindValue(const std::vector<std::string> &args, bool *wait_for_cb,
                 bool read_from_file);

  void GetContact(const std::vector<std::string> &args, bool *wait_for_cb);

  void FindNodes(const std::vector<std::string> &args, bool *wait_for_cb,
                 bool write_to_file);

  void Store50Values(const std::vector<std::string> &args,  bool *wait_for_cb);

  void StoreCallback(const int& result, const NodeId& key,
                     const bptime::time_duration &ttl);

  void FindValueCallback(int result, std::vector<std::string> values,
                         std::vector<Contact> closest_contacts,
                         Contact alternative_value_holder,
                         Contact contact_to_cache, std::string path);

  void GetContactsCallback(const int &result, Contact contact);

  void FindNodesCallback(const int &result, std::vector<Contact> contacts,
                         std::string path);

  void Store50Callback(const int& result, const std::string &key,
                       bool *arrived);
  bool ReadFile(const std::string &path, std::string *content);

  void WriteToFile(const std::string &path, const std::string &content);

  void PrintUsage();

  void ProcessCommand(const std::string &cmdline, bool *wait_for_cb);

  void PrintRpcTimings();

  std::shared_ptr<Node> node_;
  std::shared_ptr<Securifier> securifier_;
  bool result_arrived_, finish_;
  double min_succ_stores_;
};

}  // namespace kaddemo

}  // namespace kademlia

}  // namespace dht

}  // namespace maidsafe

#endif  // MAIDSAFE_DHT_TESTS_DEMO_COMMANDS_H_
