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
#include <scalopus/provider.h>
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

Provider::Provider()
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

Provider::~Provider()
{
  for (const auto& connection : connections_)
  {
    ::close(connection);
    ::shutdown(connection, 2);
  }
}

std::vector<char> Provider::readData(int connection)
{
  const size_t chunk_size = 4096;
  std::vector<char> buf(chunk_size, 0);
  size_t received { 0 };
  while(true)
  {
    ssize_t chunk_received = read(connection, &buf[received], chunk_size);

    if (chunk_received > 0)
    {
      printf("[scalopus] read %zd bytes: %.*s\n", chunk_received, static_cast<int>(chunk_received), buf.data());
    }
    received += static_cast<size_t>(chunk_received);

    if (chunk_received == chunk_size)
    {
      // read the next chunk.
      buf.resize(buf.size() + chunk_size);
      continue;
    }

    if (chunk_received == -1)
    {
      std::cerr << "[scalopus] Read error occured." << std::endl;
      ::close(connection);
      connections_.erase(connection);
      return {};
    }
    else if (chunk_received == 0)
    {
      std::cerr << "[scalopus] EOF" << std::endl;
      ::close(connection);
      connections_.erase(connection);
      return {};
    }
    std::cerr << "Total bytes from transmission.: " << received << std::endl;
    break;
  }
  return buf;
}

void Provider::work()
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
      FD_SET(connection, &write_fds);
      FD_SET(connection, &except_fds);
    }
    
    const int nfds = *std::max_element(connections_.begin(), connections_.end()) + 1;
    int response = select(nfds, &read_fds, &write_fds, &except_fds, nullptr);

    if (response == -1)
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
      std::cerr << "[scalopus] Accepted client: " << client << std::endl;
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
        std::cout << "[scalopus] read_fds on connection" << connection << std::endl;
        const auto data = readData(connection);
        if (!data.empty())
        {
          processData(connection, data);
        }
      }

      if (FD_ISSET(connection, &except_fds))
      {
        std::cout << "[scalopus] except_fds on connection" << connection << std::endl;
        ::close(connection);
        connections_.erase(connection);
      }
    }
  }
}

void Provider::processData(int connection, const std::vector<char>& incoming)
{
  printf("[scalopus] Connection %d, read %zd bytes: %.*s\n", connection, incoming.size(), static_cast<int>(incoming.size()), incoming.data());
}

void Provider::addEndpoint(Endpoint& endpoint)
{
  endpoints_[endpoint.getName()] = &endpoint;
}

}
