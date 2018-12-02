#include <iostream>
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
  if (argc == 2)
  {
    std::cout << "Sending: " << argv[1] << std::endl;
    //  endpoints.send(argv[1]);
    //  endpoints.send(argv[1]);
    endpoints.send(foo.data());
  }

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    endpoints.send(argv[1]);
  }
}
