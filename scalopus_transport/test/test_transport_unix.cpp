#include "transport_unix.h"
#include <iostream>
#include <unistd.h>

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
  // Create the server
  auto server = std::make_shared<scalopus::TransportUnix>();
  test(server->serve(), true);

  // Connect to the newly created server using our own process id.
  auto client0 = std::make_shared<scalopus::TransportUnix>();
  test(client0->connect(::getpid()), true);
  
  
  return 0;
}