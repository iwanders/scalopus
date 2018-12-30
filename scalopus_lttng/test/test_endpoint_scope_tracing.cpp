#include "scalopus_lttng/endpoint_scope_tracing.h"
#include <iostream>

template <typename A, typename B>
void test(const A& a, const B& b)
{
  bool equal = (a.size() == b.size()) && std::equal(a.begin(), a.end(), b.begin(),  [] (auto& c, auto& d)
                                                   { return c.first == d.first; });
  if (!equal)
  {
    ::exit(1);
  }
}
int main(int /* argc */, char** /* argv */)
{
  std::map<unsigned int, std::string> test_mapping;
  test_mapping[0] = "foo";
  test_mapping[1] = "bar";
  test_mapping[2] = "buz";

  auto serialized = scalopus::EndpointScopeTracing::serializeMapping(test_mapping);
  auto deserialized = scalopus::EndpointScopeTracing::deserializeMapping(serialized);

  test(test_mapping, deserialized);

  return 0;
}