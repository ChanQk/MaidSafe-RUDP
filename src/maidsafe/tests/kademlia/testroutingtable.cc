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

#include <gtest/gtest.h>
#include <boost/lexical_cast.hpp>

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/log.h"
#include "maidsafe/kademlia/contact.h"
#include "maidsafe/kademlia/kbucket.h"
#include "maidsafe/kademlia/routingtable.h"
#include "maidsafe/transport/utils.h"

namespace maidsafe {

namespace kademlia {

namespace test_routing_table {

static const boost::uint16_t K = 16;

bool TestInRange(const NodeId &key_id, const NodeId &min_range,
                 const NodeId &max_range) {
  if (min_range > key_id) {
    DLOG(INFO) << "under min range";
    DLOG(INFO) << "val " << key_id.ToStringEncoded(NodeId::kHex);
  }
  if (key_id > max_range) {
    DLOG(INFO) << "above max range";
    DLOG(INFO) << "val " << key_id.ToStringEncoded(NodeId::kHex);
  }
  return static_cast<bool>(min_range <= key_id && key_id <= max_range);
}


class TestRoutingTable : public testing::Test {
 public:
  TestRoutingTable() : cry_obj() {}
 protected:
  void SetUp() {
    cry_obj.set_symm_algorithm(crypto::AES_256);
    cry_obj.set_hash_algorithm(crypto::SHA_512);
  }
    crypto::Crypto cry_obj;
};

TEST_F(TestRoutingTable, BEH_KAD_AddContact) {
//   std::string enc_id = EncodeToHex(RandomString(512));
  NodeId holder_id(NodeId::kRandomId);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  std::string ip("127.0.0.1");
  boost::uint16_t port = 5001;
  for (int  i = 1; i <= test_routing_table::K; ++i) {
    transport::Endpoint ep(ip, port++);
    NodeId contact_id(NodeId::kRandomId);
    Contact contact(contact_id.String(), ep);
    Contact empty;
    if (!routingtable.GetContact(contact_id, &empty)) {
      EXPECT_EQ(0, routingtable.AddContact(contact));
    }
  }
}

/*
TEST_F(TestRoutingTable, FUNC_KAD_PartFilltable) {
  NodeId holder_id(NodeId::kRandomId);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  std::string ip("127.0.0.1");
  static boost::uint16_t port = 5003;

  std::list<NodeId>contacts;
  for (int i = 0; contacts.size() <=511 * test_routing_table::K ; ++i) {
    NodeId contact_id(NodeId::kRandomId);
    // seems inefficient but it is very fast so leaving like this
    contacts.push_back(contact_id);
    contacts.unique();
  }
  for (std::list<NodeId>::iterator j = contacts.begin();
       j!= contacts.end() ; ++j) {
    ++port;
    Contact contact(*j, ip, port, ip, ++port);
    // table will not be full but should only fail on full bucket [2] or
    // works [0]
    ASSERT_TRUE(routingtable.AddContact(contact) == 0 ||
                routingtable.AddContact(contact) == 2);
  }
  // One more wafer thin mint, well will be after we iterate and fill all
  // buckets TODO(dirvine#5#)
  NodeId contact_id(NodeId::kRandomId);
  Contact contact(contact_id, ip, 7777, ip, 7777);
  ASSERT_TRUE(routingtable.AddContact(contact) == 0 ||
              routingtable.AddContact(contact) == 2);
}

TEST_F(TestRoutingTable, BEH_KAD_Add_Get_Contact) {
  NodeId holder_id(NodeId::kRandomId);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  int id = RandomInt32();
  NodeId contact_id(cry_obj.Hash(boost::lexical_cast<std::string>(id),
                                     "", crypto::STRING_STRING, false));
  std::string ip("127.0.0.1");
  boost::uint16_t port(8888);
  Contact contact(contact_id, ip, port, ip, port);
  ASSERT_EQ(0, routingtable.AddContact(contact));
  Contact rec_contact;
  ASSERT_TRUE(routingtable.GetContact(contact_id, &rec_contact));
  ASSERT_TRUE(contact.Equals(rec_contact));
  DLOG(INFO) << "Recoverd contact " << rec_contact.DebugString() << std::endl;
}

TEST_F(TestRoutingTable, BEH_KAD_Add_Remove_Contact) {
  NodeId holder_id(NodeId::kRandomId);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  NodeId contact_id(NodeId::kRandomId);
  std::string ip("127.0.0.1");
  boost::uint16_t port(8888);
  Contact contact(contact_id, ip, port, ip, port);
  ASSERT_EQ(0, routingtable.AddContact(contact));

  for (int i = 0; i < kFailedRpcTolerance; ++i) {
    routingtable.RemoveContact(contact_id, false);
    Contact rec_contact;
    ASSERT_TRUE(routingtable.GetContact(contact_id, &rec_contact));
    ASSERT_EQ(i + 1, rec_contact.failed_rpc());
  }

  routingtable.RemoveContact(contact_id, false);
  Contact rec_contact1;
  ASSERT_FALSE(routingtable.GetContact(contact_id, &rec_contact1));
}

TEST_F(TestRoutingTable, BEH_KAD_Add_Remove_Add_Contact) {
  NodeId holder_id(NodeId::kRandomId);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  NodeId contact_id(NodeId::kRandomId);
  std::string ip("127.0.0.1");
  boost::uint16_t port(8888);
  Contact contact(contact_id, ip, port, ip, port);
  ASSERT_EQ(0, routingtable.AddContact(contact));

  routingtable.RemoveContact(contact_id, false);
  Contact rec_contact;
  ASSERT_FALSE(routingtable.GetContact(contact_id, &rec_contact));
}

TEST_F(TestRoutingTable, BEH_KAD_SplitKBucket) {
  if (test_routing_table::K <= 2) {  // because of force-k
    SUCCEED();
    return;
  }

  NodeId holder_id(NodeId::kRandomId);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  boost::uint32_t id[test_routing_table::K + 1];
  Contact contacts[test_routing_table::K + 1];
  id[0] = (RandomUint32() % 5000) +1;
  for (boost::uint16_t i = 0; i < test_routing_table::K + 1; ++i)
    id[i] = id[0] + i;
  std::string contact_id;
  std::string ip("127.0.0.1");
  boost::uint16_t port(8880);
  ASSERT_EQ(size_t(1), routingtable.KbucketSize());
  ASSERT_EQ(size_t(0), routingtable.Size());
  for (boost::uint16_t i = 0; i < test_routing_table::K + 1; ++i) {
    contact_id = cry_obj.Hash(boost::lexical_cast<std::string>(id[i]), "",
                              crypto::STRING_STRING, false);
    ++port;
    Contact contact(contact_id, ip, port, ip, port);
    contacts[i] = contact;
    ASSERT_EQ(0, routingtable.AddContact(contact));
  }
  ASSERT_EQ(size_t(2), routingtable.KbucketSize());
  ASSERT_EQ(test_routing_table::K + 1, routingtable.Size());
  for (boost::uint16_t i = 0; i < test_routing_table::K + 1; ++i) {
    contact_id = cry_obj.Hash(boost::lexical_cast<std::string>(id[i]), "",
                              crypto::STRING_STRING, false);
    Contact rec_contact;
    NodeId kad_ctcid(contact_id);
    ASSERT_TRUE(routingtable.GetContact(kad_ctcid, &rec_contact));
    ASSERT_TRUE(contacts[i].Equals(rec_contact));
  }
}

TEST_F(TestRoutingTable, BEH_KAD_NoSplitKBucket) {
  if (test_routing_table::K <= 2) {  // because of force-k
    SUCCEED();
    return;
  }

  std::string enc_holder_id;
  for (boost::uint16_t i = 0; i < kKeySizeBytes * 2; ++i)
    enc_holder_id += "1";
  NodeId holder_id(enc_holder_id, NodeId::kHex);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  std::string contacts_id[test_routing_table::K + 1];
  Contact contacts[test_routing_table::K + 1];
  for (boost::uint16_t i = 0; i < test_routing_table::K + 1; ++i) {
    for (boost::uint16_t j = 0; j < kKeySizeBytes * 2; ++j)
      contacts_id[i] += "d";
  }
  for (boost::uint16_t i = 0; i < test_routing_table::K + 1; ++i) {
    std::string rep;
    for (boost::uint16_t j = 0; j < i; ++j)
      rep+="f";
    contacts_id[i].replace(0, i, rep);
  }
  std::string contact_id;
  std::string ip("127.0.0.1");
  boost::uint16_t port = 8880;
  for (boost::uint16_t i = 0; i < test_routing_table::K; ++i) {
    contact_id = DecodeFromHex(contacts_id[i]);
    ++port;
    Contact contact(contact_id, ip, port, ip, port);
    contacts[i] = contact;
    ASSERT_EQ(0, routingtable.AddContact(contact));
  }

  contact_id = DecodeFromHex(contacts_id[test_routing_table::K]);
  ++port;
  Contact contact1(contact_id, ip, port, ip, port);
  ASSERT_LT(0, routingtable.AddContact(contact1));
  Contact rec_contact;
  NodeId ctc_id(contact_id);
  ASSERT_FALSE(routingtable.GetContact(ctc_id, &rec_contact));
}

TEST_F(TestRoutingTable, BEH_KAD_RefreshList_Touch) {
  NodeId min_range, max_range(NodeId::kMaxId);
  NodeId max_range1(kKeySizeBits - 1);
  NodeId max_range2(kKeySizeBits - 2);
  NodeId max_range3(kKeySizeBits - 3);
  NodeId max_range4(kKeySizeBits - 4);

  NodeId holder_id(min_range, max_range3);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  ASSERT_TRUE(max_range > max_range1);

  std::set<NodeId> ids;
  while (ids.size() < test_routing_table::K) {
    NodeId id(max_range1, max_range);
    if (id == max_range)
      continue;
    ids.insert(id);
  }
  boost::uint16_t port(8880);
  std::string ip("127.0.0.1");
  for (std::set<NodeId>::iterator i = ids.begin(); i != ids.end(); ++i) {
    Contact contact(*i, ip, port, ip, port);
    ASSERT_EQ(0, routingtable.AddContact(contact));
    ++port;
  }

  ids.clear();
  while (ids.size() < test_routing_table::K) {
    NodeId id(max_range2, max_range1);
    if (id == max_range1)
      continue;
    ids.insert(id);
  }
  for (std::set<NodeId>::iterator i = ids.begin(); i != ids.end(); ++i) {
    Contact contact(*i, ip, port, ip, port);
    ASSERT_EQ(0, routingtable.AddContact(contact));
    ++port;
  }

  ids.clear();
  while (ids.size() < test_routing_table::K) {
    NodeId id(max_range3, max_range2);
    if (id == max_range2)
      continue;
    ids.insert(id);
  }
  for (std::set<NodeId>::iterator i = ids.begin(); i != ids.end(); ++i) {
    Contact contact(*i, ip, port, ip, port);
    ASSERT_EQ(0, routingtable.AddContact(contact));
    ++port;
  }

  ids.clear();
  while (ids.size() <
         (test_routing_table::K < 2 ? 1 : test_routing_table::K / 2)) {
    NodeId id(max_range4, max_range3);
    if (id == max_range3)
      continue;
    ids.insert(id);
  }
  while (ids.size() < test_routing_table::K) {
    NodeId id(min_range, max_range4);
    if (id == max_range4)
      continue;
    ids.insert(id);
  }
  for (std::set<NodeId>::iterator i = ids.begin(); i != ids.end(); ++i) {
    Contact contact(*i, ip, port, ip, port);
    ASSERT_EQ(0, routingtable.AddContact(contact));
    ++port;
  }

  std::vector<NodeId> refresh_ids;
  routingtable.GetRefreshList(0, false, &refresh_ids);
  ASSERT_EQ(routingtable.KbucketSize(), refresh_ids.size());
  ASSERT_TRUE(TestInRange(refresh_ids[0], min_range, max_range3))
              << refresh_ids[0].ToStringEncoded(NodeId::kHex);
  ASSERT_TRUE(TestInRange(refresh_ids[1], min_range, max_range2))
              << refresh_ids[1].ToStringEncoded(NodeId::kHex);
  ASSERT_TRUE(TestInRange(refresh_ids[2], max_range3, max_range1))
              << refresh_ids[2].ToStringEncoded(NodeId::kHex);
  ASSERT_TRUE(TestInRange(refresh_ids[3], max_range2, max_range))
              << refresh_ids[3].ToStringEncoded(NodeId::kHex);
  routingtable.TouchKBucket(refresh_ids[1]);
  routingtable.TouchKBucket(refresh_ids[2]);
  refresh_ids.clear();
  routingtable.GetRefreshList(0, false, &refresh_ids);
  ASSERT_EQ(2, refresh_ids.size());
  ASSERT_TRUE(TestInRange(refresh_ids[0], min_range, max_range3));
  ASSERT_TRUE(TestInRange(refresh_ids[1], max_range2, max_range));
  refresh_ids.clear();
  routingtable.GetRefreshList(0, true, &refresh_ids);
  ASSERT_EQ(routingtable.KbucketSize(), refresh_ids.size());
}

TEST_F(TestRoutingTable, BEH_KAD_GetCloseContacts) {
  NodeId holder_id;
  NodeId min_range, max_range(NodeId::kMaxId);
  NodeId max_range1((kKeySizeBytes * 8) - 1);
  NodeId max_range2((kKeySizeBytes * 8) - 2);
  NodeId max_range3((kKeySizeBytes * 8) - 3);
  holder_id = min_range ^ max_range2;
  RoutingTable routingtable(holder_id, test_routing_table::K);
  ASSERT_TRUE(max_range > max_range1);

  std::set<NodeId> ids;
  while (ids.size() < test_routing_table::K) {
    NodeId id(max_range1, max_range);
    if (id == max_range)
      continue;
    ids.insert(id);
  }
  boost::uint16_t port(8880);
  std::string ip("127.0.0.1");
  for (std::set<NodeId>::iterator i = ids.begin(); i != ids.end(); ++i) {
    Contact contact(*i, ip, port, ip, port);
    ASSERT_EQ(0, routingtable.AddContact(contact));
    ++port;
  }

  ids.clear();
  while (ids.size() < test_routing_table::K) {
    NodeId id(max_range2, max_range1);
    if (id == max_range1)
      continue;
    ids.insert(id);
  }
  for (std::set<NodeId>::iterator i = ids.begin(); i != ids.end(); ++i) {
    Contact contact(*i, ip, port, ip, port);
    ASSERT_EQ(0, routingtable.AddContact(contact));
    ++port;
  }

  std::vector<Contact> close_nodes, ex_contacts;
  NodeId search_id(max_range1, max_range);
  routingtable.FindCloseNodes(search_id, test_routing_table::K-1,
                              ex_contacts, &close_nodes);
  ASSERT_EQ(test_routing_table::K - 1, close_nodes.size());
  for (boost::uint16_t i = 0; i < test_routing_table::K - 1; ++i)
    ASSERT_TRUE(TestInRange(close_nodes[i].node_id(), max_range1, max_range));
}

TEST_F(TestRoutingTable, BEH_KAD_ClearRoutingTable) {
  std::string enc_id("ef420cd03b20acc07f79441c6560b8e8953f0b601a968d71311abe6f1"
                     "f5feb2611692309c66f77f93ffdac4adbeddb3a28fe3b0b92d1d23592"
                     "ad9847f49580df");
  std::string ip("127.0.0.1");
  NodeId holder_id(enc_id, NodeId::kHex);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  boost::uint16_t port(8888);
  std::string ids[16];
  ids[0] = "461b69b40db1800f0b9a6cc13c257c6a06043b57841149fbbbca4dea3bcbf9119ff"
           "7cd13be0e752cf65c57b1d5abe05e5f936c9bbe1fd04fed50e97482918bae";
  ids[1] = "9e4ac275d0c0fc00fa106d9aa9db0db4e687ae2044cfbf8fcb3f371c3cb8c0d9e72"
           "583a68dc8a3ed8909b9e757d58485f654d596fd334902ca00d5ead44a87d5";
  ids[2] = "b13b8119e8d71d161b8c8e51e98326c5b82732c62f7e24be38deac1cc52ef184d78"
           "df907ff69a958a4810d7dfe22185c1798738dfee47cc194e7ca4a02d06e65";
  ids[3] = "1168eb6f58212ae41fe12fc69feb72463411754a57b83fdc7d296fcd75bdfceb539"
           "3469bf720ab0ed9f90cd10394991dcdaa133aa44a4e83b29dde66c0b716cb";
  ids[4] = "279a0166dba744ae67afff262f243e835acd795a25a122fa1c1c22a63030e013abf"
           "532fbeb4c9289e06bd478df22e255970b21f40c300bdf416b9ebcc1c8bf11";
  ids[5] = "e1f923b3defffeb6984c7c4570c99065432bd04bc0dc14fcc856bcc7472ef50a9eb"
           "15dc961e995e990f2621c43aaa259f8adfc65f74cef33a07045711073b1a1";
  ids[6] = "833b754224a2e351e40fc929d97b342f3468c0cfbea72e091370cf1a2fef7ccd4c2"
           "a0319ed0b1d808c071cc9671aa3eccafd4953c32d099b76bd2477ab9dd421";
  ids[7] = "6d5b6b3b10eebc0dfa9116059e30bf05e028fd6e4d4dfc80a3d56ce914f0d465f27"
           "baeb73d58f231530d7e72b15d2b5a59e6a2a746177d155d4e65b3a98f502c";
  ids[8] = "b9226326f7cb561e3a96e16989a1132d278a9443704c9da7b2925906d3ec80d4a21"
           "d84d5f8c52ea32626a5725cabd487bee4ded843388d65adb43112bf9b3bfa";
  ids[9] = "1f5630997a272e79d1e7091d275e5b9e115c1e2c34a5626954fba571c51b2ca29e1"
           "1dc56d60481cdf96bfdca6d0ddca016479b5ef27bba55504069e694ead957";
  ids[10] = "c05074e15f4ce0621b400d1d3da43061089294812431aaf36cab4089262eddc606"
            "bbffcd9505c277b568de4bac3e6140ce56c5e5a51b162b18127b50faa83bd7";
  ids[11] = "3dfa8a4321609f53acb492b725d9bbca32a23b14b55da092768b1d42bb36049c24"
            "7ba90ca77fa254c04e5d32be101face6285617adc32e5c92255579487f9b73";
  ids[12] = "2e6338ae33ec9a3dccc453591bc3ae3faf9ef568c66a9531d2911426d05cfe9cc8"
            "7d0922652b7d0c8544800952cc4c7669ac2eac6cd63c2da46072583b9ca835";
  ids[13] = "914aed585420dea515780906a5222e7d3848945708d497d598a41c0732a7427e75"
            "dedd698081d5ac7c09f3e2cbd5c58f466865752fac961e89e731b2c4f59c09";
  ids[14] = "b18bc05200319ce0339a68881cca8672af639ee11945188608553885330428850a"
            "4f81c8f289dd080e1f929c029810cf1ffdc82cdfd4331238f0e6a940862f1c";
  ids[15] = "a27b24b72c37e7862613b29e86502dae6f863170eb1621a04a06f909588348427b"
            "2c3bc623d7ef1bf59bd3efa010c69b19a1d8732c8512ff8510ea46176ad383";
  for (boost::uint16_t i = 0; i < 16 && i < test_routing_table::K; ++i) {
    std::string id = DecodeFromHex(ids[i]);
    Contact contact(id, ip, port + i, ip, port + i);
    ASSERT_EQ(0, routingtable.AddContact(contact));
  }
  if (test_routing_table::K > 16)
    ASSERT_EQ(16, routingtable.Size());
  else
    ASSERT_EQ(test_routing_table::K, routingtable.Size());
  routingtable.Clear();
  ASSERT_EQ(0, routingtable.Size());
}

TEST_F(TestRoutingTable, BEH_KAD_ForceK) {
  if (test_routing_table::K <= 2) {
    SUCCEED();
    return;
  }

  NodeId range1;
  NodeId range2((kKeySizeBytes * 8) - 3);
  NodeId range3((kKeySizeBytes * 8) - 2);
  NodeId range4((kKeySizeBytes * 8) - 1);
  NodeId range5(NodeId::kMaxId);
  ASSERT_TRUE(range5 > range4);
  ASSERT_TRUE(range4 > range3);
  ASSERT_TRUE(range3 > range2);
  ASSERT_TRUE(range2 > range1);
  std::string strmax_holder_id(BitToByteCount(kKeySizeBits) * 2, '0');
  strmax_holder_id[(BitToByteCount(kKeySizeBits) * 2)-1] = 'a';
  NodeId max_holder_id(strmax_holder_id, NodeId::kHex);
  NodeId holder_id(range1, max_holder_id);
  RoutingTable routingtable(holder_id, test_routing_table::K);

  // fill the first bucket
  std::string ip("127.0.0.1");
  boost::uint16_t port(8000);
  std::set<NodeId> kids;
  while (kids.size() < size_t(test_routing_table::K - 1)) {
    NodeId id(range1, range2);
    if (id == range2)
      continue;
    kids.insert(id);
  }
  std::set<NodeId>::iterator kids_it = kids.begin();
  for (boost::uint16_t i = 0; i < test_routing_table::K - 1; ++i) {
    ++port;
    Contact new_contact(*kids_it, ip, port, ip, port);
    ++kids_it;
    ASSERT_EQ(0, routingtable.AddContact(new_contact));
  }
  ASSERT_EQ(test_routing_table::K - 1, routingtable.Size());

  // fill the second bucket
  kids.clear();
  while (kids.size() < size_t(test_routing_table::K - 1)) {
    NodeId id(range4, range5);
    if (id == range5)
      continue;
    kids.insert(id);
  }
  kids_it = kids.begin();
  for (boost::uint16_t i = 0; i < test_routing_table::K - 1; ++i) {
    ++port;
    Contact new_contact(*kids_it, ip, port, ip, port);
    ++kids_it;
    ASSERT_EQ(0, routingtable.AddContact(new_contact));
  }
  ASSERT_EQ(2 * (test_routing_table::K - 1), routingtable.Size());

  // make the second bucket full with a furthest peer
  ++port;
  std::string id = range5.String();
  --id[id.size()-1];
  Contact furthest_contact(id, ip, port, ip, port);
  furthest_contact.SetLastSeenToNow();  // make sure this peer has the highest
                                        // score
  ASSERT_EQ(0, routingtable.AddContact(furthest_contact));
  ASSERT_EQ((2 * test_routing_table::K) - 1, routingtable.Size());

  // Force K will take effect when the new peer is among the K closest peers
  NodeId range4id((kKeySizeBytes * 8) - 1);
  id = range4id.String();
  ++port;
  Contact new_contact(id, ip, port, ip, port);
  ASSERT_EQ(0, routingtable.AddContact(new_contact));
  ASSERT_EQ(2 * test_routing_table::K - 1, routingtable.Size());

  // new peer which is not among K closest peers won't be accepted
  Contact new_contact1;
  ASSERT_TRUE(routingtable.GetContact(new_contact.node_id(),
                                      &new_contact1));
  ASSERT_TRUE(new_contact.Equals(new_contact1));
  Contact furthest_contact1;
  ASSERT_FALSE(routingtable.GetContact(furthest_contact.node_id(),
                                       &furthest_contact1));
  ASSERT_EQ(2, routingtable.AddContact(furthest_contact));
  ASSERT_EQ((2 * test_routing_table::K) - 1, routingtable.Size());

  // make the routingtable split further, there will be 3 buckets
  kids.clear();
  while (kids.size() < size_t(test_routing_table::K - 1)) {
    NodeId id(range3, range4);
    if (id == range4)
      continue;
    kids.insert(id);
  }
  kids_it = kids.begin();
  for (boost::uint16_t i = 0; i < test_routing_table::K - 1; ++i) {
    ++port;
    Contact new_contact(*kids_it, ip, port, ip, port);
    ASSERT_EQ(0, routingtable.AddContact(new_contact));
    ++kids_it;
  }
  ASSERT_EQ((3 * test_routing_table::K) - 2, routingtable.Size());

  // make the brother bucket of the peer full with a furthest peer
  ++port;
  id = std::string(64, 255);
  id[0] = 127;
  id[63] = static_cast<char>(254);
  Contact furthest_contact2(id, ip, port, ip, port);
  furthest_contact2.SetLastSeenToNow();  // make sure this peer has the highest
                                         // score
  ASSERT_EQ(0, routingtable.AddContact(furthest_contact2));
  ASSERT_EQ(3 * test_routing_table::K - 1, routingtable.Size());

  // Force K will take effect when the new peer is among the K cloeset peers
  id = std::string(64, 0);
  id[0] = 64;
  ++port;
  Contact new_contact2(id, ip, port, ip, port);
  ASSERT_EQ(0, routingtable.AddContact(new_contact2));
  ASSERT_EQ(3 * test_routing_table::K - 1, routingtable.Size());
  Contact new_contact3;
  ASSERT_TRUE(routingtable.GetContact(new_contact2.node_id(),
                                      &new_contact3));
  ASSERT_TRUE(new_contact2.Equals(new_contact3));
  Contact furthest_contact3;
  ASSERT_FALSE(routingtable.GetContact(furthest_contact2.node_id(),
                                       &furthest_contact3));
  // new peer which is not among K closest peers won't be accepted
  ASSERT_EQ(2, routingtable.AddContact(furthest_contact2));
  ASSERT_EQ(3 * test_routing_table::K - 1, routingtable.Size());
}

TEST_F(TestRoutingTable, BEH_KAD_GetLastSeenContact) {
  std::string enc_holder_id("7");
  for (boost::uint16_t i = 1; i < kKeySizeBytes*2; ++i)
    enc_holder_id += "1";
  NodeId holder_id(enc_holder_id, NodeId::kHex);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  std::string contacts_id_first[(test_routing_table::K/2)+1];
  std::string contacts_id_second[test_routing_table::K/2];
  Contact contacts[test_routing_table::K + 1];
  for (boost::uint16_t i = 0; i < (test_routing_table::K/2)+1; ++i) {
    for (boost::uint16_t j = 0; j < kKeySizeBytes*2; ++j)
      contacts_id_first[i] += "d";
    if (i < (test_routing_table::K/2)) {
      for (boost::uint16_t j = 0; j < kKeySizeBytes*2; ++j)
        contacts_id_second[i] += "d";
    }
  }
  for (boost::uint16_t i = 0; i < (test_routing_table::K/2)+1; ++i) {
    std::string rep;
    boost::uint16_t n =  i + 1;
    for (boost::uint16_t j = 0; j < n; ++j)
      rep+="f";
    contacts_id_first[i].replace(0, i, rep);
    contacts_id_first[i].replace(0, 1, "6");
  }
  for (boost::uint16_t i = 0; i < test_routing_table::K/2; ++i) {
    std::string rep;
    boost::uint16_t n =  i + 1;
    for (boost::uint16_t j = 0; j < n; ++j)
      rep+="f";
    contacts_id_second[i].replace(0, i+1, rep);
    contacts_id_second[i].replace(0, 1, "8");
  }
  Contact empty, result;
  result = routingtable.GetLastSeenContact(0);
  ASSERT_TRUE(empty.Equals(result));
  std::string contact_id;
  std::string ip("127.0.0.1");
  boost::uint16_t port(8880);
  for (boost::uint16_t i = 0; i < (test_routing_table::K/2)+1; ++i) {
    contact_id = DecodeFromHex(contacts_id_first[i]);
    ++port;
    Contact contact(contact_id, ip, port, ip, port);
    contacts[i] = contact;
    ASSERT_EQ(0, routingtable.AddContact(contact));
  }
  contact_id = DecodeFromHex(contacts_id_first[0]);
  Contact last_first(contact_id, ip, 8880 + 1, ip, 8880 + 1);
  result = routingtable.GetLastSeenContact(0);
  ASSERT_TRUE(last_first.Equals(result));
  for (boost::uint16_t i = 0; i < test_routing_table::K/2; ++i) {
    contact_id = DecodeFromHex(contacts_id_second[i]);
    ++port;
    Contact contact(contact_id, ip, port, ip, port);
    contacts[i] = contact;
    ASSERT_EQ(0, routingtable.AddContact(contact));
  }
  ASSERT_EQ(test_routing_table::K == 1 ? 1 : 2, routingtable.KbucketSize());
  ASSERT_EQ(2*(test_routing_table::K/2)+1, routingtable.Size());
  contact_id = DecodeFromHex(
      contacts_id_first[test_routing_table::K / 2 + 1]);
  Contact last_second(contact_id,
                           ip, 8880 + test_routing_table::K / 2 + 2,
                           ip, 8880 + test_routing_table::K / 2 + 2);
  result = routingtable.GetLastSeenContact(1);
  if (test_routing_table::K == 1)
    ASSERT_TRUE(empty.Equals(result));
  else
    ASSERT_TRUE(last_second.Equals(result));
  result = routingtable.GetLastSeenContact(0);
  ASSERT_TRUE(last_first.Equals(result));
  result = routingtable.GetLastSeenContact(2);
  ASSERT_TRUE(empty.Equals(result));
}

TEST_F(TestRoutingTable, BEH_KAD_GetKClosestContacts) {
  if (test_routing_table::K <= 4) {
    SUCCEED();
    return;
  }

  std::string holder_id_enc("7");
  for (boost::uint16_t i = 1; i < kKeySizeBytes*2; ++i)
    holder_id_enc += "1";
  std::vector<Contact> ids1(test_routing_table::K/2);
  std::vector<Contact> ids2(test_routing_table::K-2);
  NodeId holder_id(holder_id_enc, NodeId::kHex);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  std::string ip = "127.0.0.1";
  boost::uint16_t port(8000);
  for (boost::uint16_t i = 0; i < test_routing_table::K/2; ++i) {
    std::string id(kKeySizeBytes*2, '6'), rep(i, 'a'), dec_id("");
    id.replace(1, i, rep);
    dec_id = DecodeFromHex(id);
    Contact contact(dec_id, ip, port, ip, port);
    ids1[i] = contact;
    ++port;
    ASSERT_EQ(0, routingtable.AddContact(ids1[i]));
  }
  for (boost::uint16_t i = 0; i < test_routing_table::K-2; ++i) {
    std::string id(kKeySizeBytes*2, 'f'),
                rep(test_routing_table::K-1-i, '0'),
                dec_id("");
    id.replace(1, test_routing_table::K-1-i, rep);
    dec_id = DecodeFromHex(id);
    Contact contact(dec_id, ip, port, ip, port);
    ids2[i] = contact;
    ++port;
    ASSERT_EQ(0, routingtable.AddContact(ids2[i]));
    ASSERT_EQ(kKeySizeBytes * 2, id.size());
  }
  ASSERT_EQ(2, routingtable.KbucketSize());
  NodeId id1(std::string(kKeySizeBytes*2, 'e'), NodeId::kHex);
  std::vector<Contact> cts, ex;
  routingtable.FindCloseNodes(id1, test_routing_table::K, ex, &cts);
  ASSERT_EQ(test_routing_table::K, cts.size());

  // Check for no repeated values
  for (size_t i = 0; i < cts.size(); ++i) {
    for (size_t j = i+1; j < cts.size(); ++j)
      if (cts[i].Equals(cts[j])) {
        FAIL() << "Same contact in indices " << i << " and " << j;
      }
  }

  // Getting nodes that are not in cts
  for (boost::uint16_t i = 0; i < test_routing_table::K/2; ++i) {
    bool in_cts = false;
    for (size_t j = 0; j < cts.size() && !in_cts; ++j) {
      if (cts[j].Equals(ids1[i]))
        in_cts = true;
    }
    if (!in_cts)
      ex.push_back(ids1[i]);
  }
  ASSERT_FALSE(ex.empty());
  for (boost::uint16_t i = 0; i < test_routing_table::K-2; ++i) {
    bool in_cts = false;
    for (size_t j = 0; j < cts.size() && !in_cts; ++j) {
      if (cts[j].Equals(ids2[i]))
        in_cts = true;
    }
    if (!in_cts)
      ex.push_back(ids2[i]);
  }
  ASSERT_FALSE(ex.empty());
  // Checking distances
  for (size_t i = 0; i < cts.size(); ++i) {
    NodeId cts_to_id = id1 ^ cts[i].node_id();
    for (size_t j = 0; j < ex.size(); ++j) {
      NodeId ex_to_id = id1 ^ ex[j].node_id();
       ASSERT_TRUE(cts_to_id < ex_to_id);
    }
  }
}

TEST_F(TestRoutingTable, BEH_KAD_TwoKBucketsSplit) {
  std::string enc_holder_id;
  for (boost::uint16_t i = 0; i < kKeySizeBytes*2; ++i)
    enc_holder_id += "e";
  NodeId holder_id(enc_holder_id, NodeId::kHex);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  std::string contacts_id[test_routing_table::K + 1];
  Contact contacts[test_routing_table::K + 1];
  for (boost::uint16_t i = 0; i < test_routing_table::K + 1; ++i) {
    for (boost::uint16_t j = 0; j < kKeySizeBytes*2; ++j)
      contacts_id[i] += "d";
  }
  for (boost::uint16_t i = 0; i < test_routing_table::K + 1; ++i) {
    std::string rep;
    for (boost::uint16_t j = 0; j < i; ++j)
      rep+="f";
    contacts_id[i].replace(0, i, rep);
  }
  std::string contact_id;
  std::string ip("127.0.0.1");
  boost::uint16_t port = 8880;
  for (boost::uint16_t i = 0; i < test_routing_table::K + 1; ++i) {
    contact_id = DecodeFromHex(contacts_id[i]);
    ++port;
    Contact contact(contact_id, ip, port, ip, port);
    contacts[i] = contact;
    ASSERT_EQ(0, routingtable.AddContact(contact));
  }
  ASSERT_EQ(size_t(4), routingtable.KbucketSize());
  ASSERT_EQ(test_routing_table::K + 1, routingtable.Size());

  ++port;
  std::string id;
  for (boost::uint16_t j = 0; j < kKeySizeBytes*2; ++j)
    id += "e";
  contact_id.clear();
  contact_id = DecodeFromHex(id);
  Contact ctc1(contact_id, ip, port, ip, port);
  ASSERT_EQ(0, routingtable.AddContact(ctc1));
  ASSERT_EQ(size_t(5), routingtable.KbucketSize());
  ASSERT_EQ(test_routing_table::K+2, routingtable.Size());

  id.clear();
  for (boost::uint16_t j = 0; j < kKeySizeBytes*2; ++j)
    id += "2";
  ++port;
  contact_id.clear();
  contact_id = DecodeFromHex(id);
  Contact ctc2(contact_id, ip, port, ip, port);
  ASSERT_EQ(0, routingtable.AddContact(ctc2));

  ASSERT_EQ(size_t(5), routingtable.KbucketSize());
  ASSERT_EQ(test_routing_table::K+3, routingtable.Size());
  for (boost::uint16_t i = 0; i < test_routing_table::K; ++i) {
    NodeId id_ctc(contacts_id[i], NodeId::kHex);
    Contact rec_contact;
    ASSERT_TRUE(routingtable.GetContact(id_ctc, &rec_contact));
    ASSERT_TRUE(contacts[i].Equals(rec_contact));
  }
  Contact rec_ctc;
  ASSERT_TRUE(routingtable.GetContact(ctc1.node_id(), &rec_ctc));
  ASSERT_TRUE(ctc1.Equals(rec_ctc));
  ASSERT_TRUE(routingtable.GetContact(ctc2.node_id(), &rec_ctc));
  ASSERT_TRUE(ctc2.Equals(rec_ctc));
}

TEST_F(TestRoutingTable, BEH_KAD_GetFurthestNodes) {
  NodeId holder_id(NodeId::kRandomId);
  RoutingTable routingtable(holder_id, test_routing_table::K);
  std::string ip("127.0.0.");
  boost::uint16_t port = 5001;
  for (boost::uint16_t i = 1; i < 254; ++i) {
    NodeId contact_id(NodeId::kRandomId);
    Contact contact(contact_id, ip + IntToString(i), port + i,
                         ip + IntToString(i), port + i);
    Contact empty;
    if (!routingtable.GetContact(contact_id, &empty)) {
      routingtable.AddContact(contact);
    }
  }
  std::vector<Contact> exclude_contacts;
  std::vector<Contact> all_nodes;
  routingtable.GetFurthestContacts(holder_id, -1, exclude_contacts,
                                   &all_nodes);
  ASSERT_EQ(routingtable.Size(), all_nodes.size());
  for (size_t n = 0; n < all_nodes.size() - 1; ++n) {
    const NodeId k1 = holder_id ^ all_nodes[n].node_id();
    const NodeId k2 = holder_id ^ all_nodes[n+1].node_id();
    ASSERT_TRUE(k1 > k2) << "Failed on " << n << std::endl;
  }

  boost::int8_t count(static_cast<boost::int8_t>(test_routing_table::K));
  if (routingtable.Size() <= test_routing_table::K)
    count = static_cast<boost::int8_t>(test_routing_table::K / 2);

  std::vector<Contact> k_furthest_nodes;
  routingtable.GetFurthestContacts(holder_id, count, exclude_contacts,
                                   &k_furthest_nodes);
  ASSERT_EQ(static_cast<size_t>(count), k_furthest_nodes.size());
  for (size_t a = 0; a < k_furthest_nodes.size() - 1; ++a) {
    const NodeId k1 = holder_id ^ k_furthest_nodes[a].node_id();
    const NodeId k2 = holder_id ^ k_furthest_nodes[a+1].node_id();
    ASSERT_TRUE(k1 > k2) << "Failed on " << a << std::endl;
  }

  for (size_t y = 0; y < k_furthest_nodes.size(); ++y) {
    const NodeId k1 = k_furthest_nodes[y].node_id();
    const NodeId k2 = all_nodes[y].node_id();
    ASSERT_TRUE(k1 == k2) << "Failed on " << y << std::endl
                          << "1: "<< k1.ToStringEncoded(NodeId::kHex)
                          << std::endl
                          << "2: " << k2.ToStringEncoded(NodeId::kHex)
                          << std::endl;
  }
}

*/
}  // namespace test_routing_table

}  // namespace kademlia

}  // namespace maidsafe
