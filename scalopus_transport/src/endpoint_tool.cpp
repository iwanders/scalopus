#include <iostream>
#include <cstring>
#include "consumer.h"

int main(int argc, char** argv)
{
  const auto providers = scalopus::Consumer::getProviders();
  for (const auto& pid : providers)
  {
    std::cout << pid << std::endl;
  }

  scalopus::Consumer endpoints;
  endpoints.connect(providers.front());

  std::vector<char> foo(4095*2, 20);

  endpoints.send("foo", foo);

  if (argc == 2)
  {
    std::vector<char> input(argv[1], argv[1] + std::strlen(argv[1]));
    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      endpoints.send("foo", input);
    }
  }
}
