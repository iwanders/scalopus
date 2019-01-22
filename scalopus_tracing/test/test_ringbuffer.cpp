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

  // We should be able to add to it.
  test(ring.push(1), true);
  test(ring.push(2), true);
  test(ring.push(3), false);  // queue is full.

  test(ring.pop(res), true);
  test(res, 1);
  test(ring.pop(res), true);
  test(res, 2);
  test(ring.pop(res), false);  // popping on empty queue

  test(ring.push(2), true);
  test(ring.pop(res), true);
  test(res, 2);
  test(ring.push(1), true);
}

int main(int /* argc */, char** /* argv */)
{
  scalopus::SPSCRingBuffer<std::vector<int>> ring_vector{ std::vector<int>(3, 0) };
  run_tests(ring_vector);
  scalopus::SPSCRingBuffer<std::array<int, 3>> ring_array{ std::array<int, 3>() };
  run_tests(ring_array);
  return 0;
}