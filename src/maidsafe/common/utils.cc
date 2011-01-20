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

#include "maidsafe/common/utils.h"
#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread/mutex.hpp>
#include <ctype.h>
#include <maidsafe/cryptopp/integer.h>
#include <maidsafe/cryptopp/osrng.h>
#include <maidsafe/cryptopp/base32.h>
#include <maidsafe/cryptopp/base64.h>
#include <maidsafe/cryptopp/hex.h>
#include <algorithm>
#include <limits>
#include <string>
#include "maidsafe/common/log.h"
#include "maidsafe/transport/network_interface.h"

namespace maidsafe {

const boost::posix_time::ptime kMaidSafeEpoch(
    boost::gregorian::date(2000, 1, 1));

CryptoPP::AutoSeededX917RNG<CryptoPP::AES> g_random_number_generator;
boost::mutex g_random_number_generator_mutex;

boost::int32_t RandomInt32() {
  boost::int32_t result(0);
  bool success = false;
  while (!success) {
    boost::mutex::scoped_lock lock(g_random_number_generator_mutex);
    CryptoPP::Integer rand_num(g_random_number_generator, 32);
    if (rand_num.IsConvertableToLong()) {
      result = static_cast<boost::int32_t>(
               rand_num.AbsoluteValue().ConvertToLong());
      success = true;
    }
  }
  return result;
}

boost::uint32_t RandomUint32() {
  return static_cast<boost::uint32_t>(RandomInt32());
}

std::string IntToString(const int &value) {
  return boost::lexical_cast<std::string>(value);
}

std::string RandomString(const size_t &length) {
  std::string random_string;
  random_string.reserve(length);
  while (random_string.size() < length) {
#ifdef MAIDSAFE_APPLE
     size_t iter_length = (length - random_string.size()) < 65536U ?
                          (length - random_string.size()) : 65536U;
#else
    size_t iter_length = std::min(length - random_string.size(), size_t(65536));
#endif
    boost::scoped_array<byte> random_bytes(new byte[iter_length]);
    {
      boost::mutex::scoped_lock lock(g_random_number_generator_mutex);
      g_random_number_generator.GenerateBlock(random_bytes.get(), iter_length);
    }
    std::string random_substring;
    CryptoPP::StringSink string_sink(random_substring);
    string_sink.Put(random_bytes.get(), iter_length);
    random_string += random_substring;
  }
  return random_string;
}

std::string RandomAlphaNumericString(const size_t &length) {
  std::string random_string(RandomString(length));
  for (std::string::iterator it = random_string.begin();
       it != random_string.end(); ++it) {
    *it = (*it + 128) % 122;
    if (48 > *it)
      *it += 48;
    if ((57 < *it) && (65 > *it))
      *it += 7;
    if ((90 < *it) && (97 > *it))
      *it += 6;
  }
  return random_string;
}

std::string EncodeToHex(const std::string &non_hex_input) {
  std::string hex_output;
  CryptoPP::StringSource(non_hex_input, true,
      new CryptoPP::HexEncoder(new CryptoPP::StringSink(hex_output), false));
  return hex_output;
}

std::string EncodeToBase64(const std::string &non_base64_input) {
  std::string base64_output;
  CryptoPP::StringSource(non_base64_input, true, new CryptoPP::Base64Encoder(
      new CryptoPP::StringSink(base64_output), false, 255));
  return base64_output;
}

std::string EncodeToBase32(const std::string &non_base32_input) {
  std::string base32_output;
  CryptoPP::StringSource(non_base32_input, true, new CryptoPP::Base32Encoder(
      new CryptoPP::StringSink(base32_output), false));
  return base32_output;
}

std::string DecodeFromHex(const std::string &hex_input) {
  std::string non_hex_output;
  CryptoPP::StringSource(hex_input, true,
      new CryptoPP::HexDecoder(new CryptoPP::StringSink(non_hex_output)));
  return non_hex_output;
}

std::string DecodeFromBase64(const std::string &base64_input) {
  std::string non_base64_output;
  CryptoPP::StringSource(base64_input, true,
      new CryptoPP::Base64Decoder(new CryptoPP::StringSink(non_base64_output)));
  return non_base64_output;
}

std::string DecodeFromBase32(const std::string &base32_input) {
  std::string non_base32_output;
  CryptoPP::StringSource(base32_input, true,
      new CryptoPP::Base32Decoder(new CryptoPP::StringSink(non_base32_output)));
  return non_base32_output;
}

boost::uint32_t GetEpochTime() {
  boost::posix_time::ptime
      t(boost::posix_time::microsec_clock::universal_time());
  return static_cast<boost::uint32_t>((t - kMaidSafeEpoch).total_seconds());
}

boost::uint64_t GetEpochMilliseconds() {
  boost::posix_time::ptime
      t(boost::posix_time::microsec_clock::universal_time());
  return static_cast<boost::uint64_t>((t - kMaidSafeEpoch).total_milliseconds());
}

boost::uint64_t GetEpochNanoseconds() {
  boost::posix_time::ptime
      t(boost::posix_time::microsec_clock::universal_time());
  return static_cast<boost::uint64_t>((t - kMaidSafeEpoch).total_nanoseconds());
}

boost::uint32_t GenerateNextTransactionId(const boost::uint32_t &id) {
  const boost::uint32_t kMaxId = 2147483645;
  if (id == 0) {
    return (RandomUint32() % kMaxId) + 1;
  } else {
    return (id % kMaxId) + 1;
  }
}

}  // namespace maidsafe
