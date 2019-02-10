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

template <typename A, typename B>
void test(const A& a, const B& b)
{
  if (a != b)
  {
    std::cerr << "a (" << a << ") != b (" << b << ")" << std::endl;
    exit(1);
  }
}

template <typename RingType>
void run_tests(RingType& ring)
{
  int res;
  // queue should start empty:
  test(ring.pop(res), false);  // Check if popping an empty queue fails.
  test(ring.empty(), true);
  test(ring.size(), 0);

  // We should be able to add to it.
  test(ring.push(1), true);
  test(ring.size(), 1);
  test(ring.push(2), true);
  test(ring.size(), 2);
  test(ring.push(3), false);  // queue is full.
  test(ring.size(), 2);

  test(ring.pop(res), true);
  test(res, 1);
  test(ring.size(), 1);
  test(ring.pop(res), true);
  test(res, 2);
  test(ring.size(), 0);
  test(ring.pop(res), false);  // popping on empty queue
  test(ring.empty(), true);

  test(ring.push(2), true);
  test(ring.size(), 1);
  test(ring.pop(res), true);
  test(ring.size(), 0);
  test(res, 2);
  test(ring.push(1), true);
}

template <typename Ringtype>
void test_readinto(Ringtype& ring)
{
  test(ring.push(1), true);
  test(ring.push(2), true);
  test(ring.push(3), true);
  test(ring.push(4), true);

  // pop the four entries into a buffer.
  std::vector<int> consumed_buffer{};
  consumed_buffer.reserve(4);
  test(ring.pop_into(std::back_inserter(consumed_buffer), 4), 4);
  test(consumed_buffer[0], 1);
  test(consumed_buffer[1], 2);
  test(consumed_buffer[2], 3);
  test(consumed_buffer[3], 4);

  // Now add 6, this wraps around the ringbuffer.
  test(ring.push(1), true);
  test(ring.push(2), true);
  test(ring.push(3), true);
  test(ring.push(4), true);
  test(ring.push(5), true);
  test(ring.push(6), true);
  test(ring.size(), 6);

  // pop that into consumed.
  consumed_buffer.clear();
  test(ring.pop_into(std::back_inserter(consumed_buffer), 6), 6);
  test(consumed_buffer[0], 1);
  test(consumed_buffer[1], 2);
  test(consumed_buffer[2], 3);
  test(consumed_buffer[3], 4);
  test(consumed_buffer[4], 5);
  test(consumed_buffer[5], 6);

  // Add 6 more entries.
  test(ring.push(1), true);
  test(ring.push(2), true);
  test(ring.push(3), true);
  test(ring.push(4), true);
  test(ring.push(5), true);
  test(ring.push(6), true);

  // Check limited read_count by reading just 2 entries.
  test(ring.pop_into(std::back_inserter(consumed_buffer), 2), 2);
  test(ring.size(), 4);
  test(consumed_buffer.size(), 8);

  // Check reading into a fixed size array.
  std::array<int, 3> my_buffer;
  test(ring.pop_into(my_buffer.begin(), my_buffer.size()), 3);
  test(my_buffer[0], 3);
  test(my_buffer[1], 4);
  test(my_buffer[2], 5);
}

int main(int /* argc */, char** /* argv */)
{
  scalopus::SPSCRingBuffer<std::vector<int>> ring_vector{ std::vector<int>(3, 0) };
  run_tests(ring_vector);
  scalopus::SPSCRingBuffer<std::array<int, 3>> ring_array{ std::array<int, 3>() };
  run_tests(ring_array);


  scalopus::SPSCRingBuffer<std::vector<int>> ring_vector_read_into{ std::vector<int>(8, 0) };
  test_readinto(ring_vector_read_into);
  scalopus::SPSCRingBuffer<std::array<int, 8>> ring_array_read_into{ std::array<int, 8>() };
  test_readinto(ring_array_read_into);
  return 0;
}