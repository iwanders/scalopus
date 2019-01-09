/*
  Copyright (c) 2019, Ivor Wanders
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
#include "scalopus_lttng/endpoint_scope_tracing.h"
#include "scalopus_lttng/scope_tracing.h"
#include <scalopus_transport/transport_mock.h>

template <typename A, typename B>
void test_map(const A& a, const B& b)
{
  bool equal = (a.size() == b.size()) &&
               std::equal(a.begin(), a.end(), b.begin(), [](auto& c, auto& d) { return c.first == d.first; });
  if (!equal)
  {
    ::exit(1);
  }
}

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
  scalopus::EndpointScopeTracing::TraceIdMap test_mapping;
  test_mapping[0] = "foo";
  test_mapping[1] = "bar";
  test_mapping[2] = "buz";

  for (const auto& id_name : test_mapping)
  {
    scalopus::ScopeTraceTracker::getInstance().insert(id_name.first, id_name.second);
  }

  // Create the endpoint.
  auto server_endpoint = std::make_shared<scalopus::EndpointScopeTracing>();

  auto factory = std::make_shared<scalopus::TransportMockFactory>();
  auto server = factory->serve();
  server->addEndpoint(server_endpoint);
  auto client = factory->connect(server);

  // Add the endpoint to the client.
  auto client_endpoint = std::make_shared<scalopus::EndpointScopeTracing>();
  client_endpoint->setTransport(client);

  auto retrieved_mapping = client_endpoint->mapping();

  // there should be one entry in it, our current process id, that map should be identical to test_mapping.
  test(retrieved_mapping.size(), 1u);
  test_map(retrieved_mapping.begin()->second, test_mapping);

  return 0;
}