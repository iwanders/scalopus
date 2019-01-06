#include <iostream>
#include "scalopus_lttng/endpoint_scope_tracing.h"

template <typename A, typename B>
void test(const A& a, const B& b)
{
  bool equal = (a.size() == b.size()) &&
               std::equal(a.begin(), a.end(), b.begin(), [](auto& c, auto& d) { return c.first == d.first; });
  if (!equal)
  {
    ::exit(1);
  }
}
int main(int /* argc */, char** /* argv */)
{
  scalopus::EndpointScopeTracing::TraceIdMap test_mapping;
  test_mapping[0] = "foo";
  test_mapping[1] = "bar";
  test_mapping[2] = "buz";

  scalopus::EndpointScopeTracing::ProcessTraceMap process_mapping;
  process_mapping[5] = test_mapping;
  process_mapping[11] = test_mapping;
  process_mapping[10] = { { 3, "foo" } };

  auto serialized = scalopus::EndpointScopeTracing::serializeMapping(process_mapping);
  auto deserialized = scalopus::EndpointScopeTracing::deserializeMapping(serialized);

  test(test_mapping, deserialized);
  for (auto& pid_map : test_mapping)
  {
    test(process_mapping[pid_map.first], deserialized[pid_map.first]);
  }

  return 0;
}