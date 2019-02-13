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
#include <vector>
#include "spsc_ringbuffer.h"
#include "cbor.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using Data = std::vector<std::uint8_t>;


template <typename A, typename B>
void test(const A& a, const B& b)
{
  if (a != b)
  {
    std::cerr << "a (" << a << ") != b (" << b << ")" << std::endl;
    exit(1);
  }
}

//! Timepoint of that clock.
using TimePoint = uint64_t;
//! Trace event as it is stored in the ringbuffer.
using ScopeTraceEvent = std::tuple<TimePoint, unsigned int, uint8_t>;
//! The container that backs the ringbuffer.
using EventContainer = std::vector<ScopeTraceEvent>;

int main(int /* argc */, char** /* argv */)
{
  json events;
  events["pid"] = std::uint64_t{1337};
  events["foo"] = EventContainer{{1549943736559416, 20, 30}, {1549943736559417, 655351, 60}};
  auto z = json::to_cbor(events);
  std::cout << cbor::hexdump(z) << std::endl;

  std::map<std::string, cbor::cbor_object> mymap;
  mymap["pid"] = cbor::cbor_object::make(std::uint64_t(1337));
  mymap["foo"] = cbor::cbor_object::make(EventContainer{{1549943736559416, 20, 30}, {1549943736559417, 655351, 60}});
  
  Data output;
  cbor::serialize(mymap, output);
  std::cout << cbor::hexdump(output) << std::endl;

  //  std::tuple<uint32_t, uint32_t, uint32_t> foo{1,2,3};
  std::vector<std::tuple<unsigned long, EventContainer>> foo;
  foo.push_back({1549943736559416, EventContainer{{1549943736559416, 20, 30}, {1549943736559417, 655351, 5}}});
  //  foo.push_back({1, EventContainer{{1, 2, 3}}});
  output.resize(0);
  cbor::serialize(foo, output);
  std::cout << cbor::hexdump(output) << std::endl;

  //  std::vector<std::tuple<int, int>> buz;
  //  buz = {{1,2}, {3,4}};
  
  return 0;
}