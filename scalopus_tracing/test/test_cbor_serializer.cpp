/*
  Copyright (c) 2018-2019, Ivor Wanders
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of the author nor the names of contributors may be used to
    endorse or promote products derived from this software without specific
    prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <array>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>
#include "cbor.h"
#include "spsc_ringbuffer.h"

using json = nlohmann::json;
using Data = std::vector<std::uint8_t>;

template <typename A, typename B>
void test(const A& a, const B& b)
{
  if (a != b)
  {
    std::cerr << "a (\n" << a << ") != b (\n" << b << ")" << std::endl;
    exit(1);
  }
}

int main(int /* argc */, char** /* argv */)
{
  //! Timepoint of that clock.
  using TimePoint = uint64_t;
  //! Trace event as it is stored in the ringbuffer.
  using ScopeTraceEvent = std::tuple<TimePoint, unsigned int, uint8_t>;
  //! The container that backs the ringbuffer.
  using EventContainer = std::vector<ScopeTraceEvent>;
  using EventMap = std::map<unsigned long, EventContainer>;

  // This is what we need to match.
  json events;
  events["pid"] = std::uint64_t{ 1337 };
  events["events"] =
      EventMap{ { 141414, EventContainer{ { 1549943736559416, 20, 30 }, { 1549943736559417, 655351, 60 } } } };
  const auto json_events_as_cbor = json::to_cbor(events);
  std::cout << cbor::hexdump(json_events_as_cbor) << std::endl;

  // json doesn't allow non-string keys in maps, so those are packed as lists of (key, value) pairs.

  // Create map of string , cbor_object data
  std::map<std::string, cbor::cbor_object> my_event_data;
  // Add the pid entry.
  my_event_data["pid"] = cbor::cbor_object::make(static_cast<unsigned long>(1337));

  // Create the 'map' with a vector of tuples.
  std::vector<std::tuple<unsigned long, EventContainer>> faux_event_map;
  faux_event_map.push_back(
      { 141414, EventContainer{ { 1549943736559416, 20, 30 }, { 1549943736559417, 655351, 60 } } });
  my_event_data["events"] = cbor::cbor_object::make(faux_event_map);

  // Serialize the map of cbor entries into the data.
  Data events_as_cbor;
  cbor::serialize(my_event_data, events_as_cbor);
  std::cout << cbor::hexdump(events_as_cbor) << std::endl;  // print it

  // test it! If this passes we can write data identical to nlohmann::json's to_cbor, but then lots faster.
  test(cbor::hexdump(json_events_as_cbor), cbor::hexdump(events_as_cbor));

  // some values from https://github.com/cbor/test-vectors/blob/master/appendix_a.json
  // Test map
  {
    std::map<unsigned int, unsigned int> input{ { 1, 2 }, { 3, 4 } };
    Data result = { 0xa2, 0x01, 0x02, 0x03, 0x04 };
    Data cbor_representation;
    cbor::serialize(input, cbor_representation);
    test(cbor::hexdump(result), cbor::hexdump(cbor_representation));
  }
  // test string map
  {
    std::map<std::string, std::string> input{ { "a", "A" }, { "b", "B" }, { "c", "C" }, { "d", "D" }, { "e", "E" } };
    Data result = { 0xa5, 0x61, 0x61, 0x61, 0x41, 0x61, 0x62, 0x61, 0x42, 0x61, 0x63,
                    0x61, 0x43, 0x61, 0x64, 0x61, 0x44, 0x61, 0x65, 0x61, 0x45 };
    Data cbor_representation;
    cbor::serialize(input, cbor_representation);
    test(cbor::hexdump(result), cbor::hexdump(cbor_representation));
  }

  // test tuple
  {
    std::tuple<unsigned int, unsigned int> input{ 1, 2 };
    Data result = { 0x82, 0x01, 0x02 };
    Data cbor_representation;
    cbor::serialize(input, cbor_representation);
    test(cbor::hexdump(result), cbor::hexdump(cbor_representation));
  }

  // test pair
  {
    std::pair<unsigned int, unsigned int> input{ 1, 2 };
    Data result = { 0x82, 0x01, 0x02 };
    Data cbor_representation;
    cbor::serialize(input, cbor_representation);
    test(cbor::hexdump(result), cbor::hexdump(cbor_representation));
  }
  return 0;
}