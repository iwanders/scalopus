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
#include "transport_server_unix.h"
#include "protocol.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <iostream>
#include <algorithm>


namespace scalopus
{

TransportServerUnix::TransportServerUnix()
{
  // Create the server socket to work with.
  server_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);

  if (server_fd_ == -1)
  {
    std::cerr << "[scalopus] Could not create socket." << std::endl;
    // Should we continue if this happens?
  }

  // Create the socket struct.
  struct sockaddr_un socket_config;
  std::memset(&socket_config, 0, sizeof(socket_config));
  socket_config.sun_family = AF_UNIX;

  // Copy the desired unix domain socket name into the struct.
  std::stringstream ss;
  ss << "" << ::getpid() << "_scalopus";
  std::strncpy(socket_config.sun_path + 1, ss.str().c_str(), sizeof(socket_config.sun_path) - 2);

  // Obtain the real path size such that we don't set a path that's too long.
  std::size_t path_length = offsetof(struct sockaddr_un, sun_path) + strlen(socket_config.sun_path + 1) + 1;

  // Bind the unix socket on the path we created.
  if (bind(server_fd_, reinterpret_cast<sockaddr*>(&socket_config), static_cast<unsigned int>(path_length)) == -1)
  {
    std::cerr << "[scalopus] Could not bind socket." << std::endl;
  }

  // Listen for connections, with a queue of five.
  if (listen(server_fd_, 5) == -1)
  {
    std::cerr << "[scalopus] Could not start listening for connections." << std::endl;
  }

  std::cerr << "Server: " << server_fd_ << std::endl;
  connections_.insert(server_fd_);

  // If we get here, we are golden, we got a working unix domain socket and can start our worker thread.
  if (server_fd_)
  {
    thread_ = std::thread([this]() { work(); });
  }
  else
  {
    running_ = false;
  }
}

TransportServerUnix::~TransportServerUnix()
{
  for (const auto& connection : connections_)
  {
    ::close(connection);
    ::shutdown(connection, 2);
  }
}

void TransportServerUnix::work()
{
  fd_set read_fds;
  fd_set write_fds;
  fd_set except_fds;
  while (running_)
  {
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);
    for (const auto& connection : connections_)
    {
      FD_SET(connection, &read_fds);
      //  FD_SET(connection, &write_fds);  // We will just block when writing, that's fine.
      FD_SET(connection, &except_fds);
    }
    
    const int nfds = *std::max_element(connections_.begin(), connections_.end()) + 1;
    int select_result = select(nfds, &read_fds, &write_fds, &except_fds, nullptr);

    if (select_result == -1)
    {
      std::cerr << "[scalopus]: Failure occured on select." << std::endl;
    }

    // Handle server stuff, accept new connection:
    if (FD_ISSET(server_fd_, &read_fds))
    {
      int client { 0 };
      client = accept(server_fd_, nullptr, nullptr);
      if (client == -1)
      {
        std::cerr << "[scalopus] Could not accept client." << std::endl;
        continue;
      }
      connections_.insert(client);
    }

    if (FD_ISSET(server_fd_, &except_fds))
    {
      std::cout << "[scalopus] Exception on server " << std::endl;
    }

    // else; check everything.
    const auto copy_of_connections = connections_;
    for (const auto& connection : copy_of_connections)
    {
      if (connection == server_fd_)
      {
        continue;
      }
      
      if (FD_ISSET(connection, &read_fds))
      {
        protocol::Msg request;
        bool result = protocol::receive(connection, request);
        if (result)
        {
          protocol::Msg response;
          if (processMsg(request, response))
          {
            // send response...
            protocol::send(connection, response);
          }
        }
        else
        {
          ::close(connection);
          ::shutdown(connection, 2);
          connections_.erase(connection);
        }
      }

      if (FD_ISSET(connection, &except_fds))
      {
        ::close(connection);
        ::shutdown(connection, 2);
        connections_.erase(connection);
      }
    }
  }
}

bool TransportServerUnix::processMsg(const protocol::Msg& request, protocol::Msg& response)
{
  response.endpoint = request.endpoint;
  // Check if we have this endpoint.
  const auto it = endpoints_.find(request.endpoint);
  if (it != endpoints_.end())
  {
    // Let the endpoint handle the data and if necessary respond.
    return it->second->handle(*this, request.data, response.data);
  }
  return false;
}


std::unique_ptr<TransportServer> transportServerUnix()
{
  return std::make_unique<TransportServerUnix>();
}

}
