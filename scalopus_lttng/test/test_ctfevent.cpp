#include "scalopus_lttng/ctfevent.h"
#include <iostream>

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
  std::string line{"[1544361620.739021131] eagle scalopus_scope_id:exit: { cpu_id = 2 }, { vpid = 14897, pthread_id = 139688084124608 }, { id = 4144779573 }"};
  scalopus::CTFEvent event{line};
  std::cout << event << std::endl;
  test(event.time(), 1544361620.739021131);
  test(event.tid(), 139688084124608U);
  test(event.pid(), 14897U);
  test(event.domain(), "scalopus_scope_id");
  test(event.name(), "exit");
  test(event.eventData().at("id"), 4144779573U);
  return 0;
}