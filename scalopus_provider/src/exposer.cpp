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
#include <scalopus/exposer.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <iostream>

namespace scalopus
{

Exposer::Exposer()
{
  fd_ = socket(AF_UNIX, SOCK_STREAM, 0);

  if (fd_ == -1)
  {
    std::cerr << "[scalopus] Could not create socket." << std::endl;
    // Should we continue if this happens?
  }

  // Create the socket struct.
  struct sockaddr_un socket_config;
  std::memset(&socket_config, 0, sizeof(socket_config));
  socket_config.sun_family = AF_UNIX;

  std::stringstream ss;
  ss << "scalopus_" << ::getpid();
  std::strncpy(socket_config.sun_path + 1, ss.str().c_str(), sizeof(socket_config.sun_path) - 2);

  std::size_t path_length = offsetof(struct sockaddr_un, sun_path) + strlen(socket_config.sun_path + 1) + 1;
  if (bind(fd_, reinterpret_cast<sockaddr*>(&socket_config), path_length) == -1)
  {
    std::cerr << "[scalopus] Could not bind socket." << std::endl;
  }

  if (listen(fd_, 5) == -1)
  {
    std::cerr << "[scalopus] Could not start listening for connections." << std::endl;
  }

  // If we get here, we are golden, we got a working unix domain socket and can start our worker thread.
}

void Exposer::work()
{
  while (running_)
  {
    int client { 0 };
    client = accept(fd_, NULL, NULL);
    if (client == -1)
    {
      perror("accept error");
      continue;
    }
    int received { 0 };
    std::array<char, 4096> buf;
    received = read(client, buf.data(), buf.size());

    while (received > 0)
    {
      printf("read %u bytes: %.*s\n", received, received, buf.data());
    }

    if (received == -1)
    {
      std::cerr << "[scalopus] Read error occured." << std::endl;
    }
    else if (received == 0)
    {
      printf("EOF\n");
      close(client);
    }
  }
}

}
