/*
  Copyright (c) 2018, Ivor Wanders
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

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
#include <iostream>
#include "scalopus_lttng/ctfevent.h"

template <typename A, typename B>
void test(const A& a, const B& b)
{
  if (a != b)
  {
    std::cerr << "a (" << a << ") != b (" << b << ")" << std::endl;
    exit(1);
  }
}
int main(int /* argc */, char** /* argv */)
{
  std::string line{ "[1544361620.739021131] eagle scalopus_scope_id:exit: { cpu_id = 2 }, { vpid = 14897, pthread_id = "
                    "139688084124608 }, { id = 4144779573 }" };
  scalopus::CTFEvent event{ line };
  std::cout << event << std::endl;
  test(event.time(), 1544361620.739021131);
  test(event.tid(), 139688084124608U);
  test(event.pid(), 14897U);
  test(event.domain(), "scalopus_scope_id");
  test(event.name(), "exit");
  test(event.eventData().at("id"), 4144779573U);
  return 0;
}