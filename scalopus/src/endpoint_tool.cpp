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

  if (argc == 2)
  {
    std::cout << "Sending: " << argv[1] << std::endl;
    endpoints.send(argv[1]);
  }
}
