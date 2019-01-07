#include <iostream>
#include "scalopus_transport/transport_mock.h"
#include "scalopus_general/endpoint_process_info.h"
#include "scalopus_general/thread_naming.h"

#include <sys/types.h>
#include <unistd.h>

template <typename A, typename B>
void test(const A& a, const B& b)
{
  if (a != b)
  {
    ::exit(1);
  }
}
int main(int /* argc */, char** /* argv */)
{
  // Set the thread name.
  TRACE_THREAD_NAME("my_thread");

  // Create the endpoint.
  auto server_info = std::make_shared<scalopus::EndpointProcessInfo>();

  // Assign a process name.
  server_info->setProcessName("Fooo");

  // Create mock server and client.
  auto server = scalopus::transportServerMock();
  server->addEndpoint(server_info);

  auto client = scalopus::transportClientMock(server);
  // Add the endpoint to the client.
  auto client_info = std::make_shared<scalopus::EndpointProcessInfo>();
  client_info->setTransport(client);

  // Retrieve the servers' properties.
  auto process_info = client_info->processInfo();

  // Check if the values are correct.
  test(process_info.name, std::string("Fooo"));
  test(process_info.pid, static_cast<unsigned long>(::getpid()));
  test(process_info.threads[static_cast<unsigned long>(pthread_self())], "my_thread");
  return 0;
}