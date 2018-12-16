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
#include "transport_unix.h"
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include "protocol.h"

namespace scalopus
{
TransportUnix::TransportUnix()
{
}

bool TransportUnix::serve()
{
  // Create the server socket to work with.
  server_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);

  if (server_fd_ == -1)
  {
    std::cerr << "[scalopus] Could not create socket." << std::endl;
    return false;
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
  std::size_t path_length = sizeof(socket_config.sun_family) + strlen(socket_config.sun_path + 1) + 1;

  // Bind the unix socket on the path we created.
  if (bind(server_fd_, reinterpret_cast<sockaddr*>(&socket_config), static_cast<unsigned int>(path_length)) == -1)
  {
    std::cerr << "[scalopus] Could not bind socket." << std::endl;
    return false;
  }

  // Listen for connections, with a queue of five.
  if (listen(server_fd_, 5) == -1)
  {
    std::cerr << "[scalopus] Could not start listening for connections." << std::endl;
    return false;
  }

  std::cerr << "Server: " << server_fd_ << std::endl;
  connections_.insert(server_fd_);

  // If we get here, we are golden, we got a working unix domain socket and can start our worker thread.
  running_ = true;
  thread_ = std::thread([this]() { work(); });
  
  return true;
}

bool TransportUnix::connect(std::size_t pid, const std::string& suffix)
{
  client_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);

  if (client_fd_ == -1)
  {
    std::cerr << "[scalopus] Could not create socket." << std::endl;
    return false;
  }

  // Create the socket struct.
  struct sockaddr_un socket_config;
  std::memset(&socket_config, 0, sizeof(socket_config));
  socket_config.sun_family = AF_UNIX;

  std::stringstream ss;
  ss << "" << pid << suffix;
  std::strncpy(socket_config.sun_path + 1, ss.str().c_str(), sizeof(socket_config.sun_path) - 2);

  std::size_t path_length = sizeof(socket_config.sun_family) + strlen(socket_config.sun_path + 1) + 1;
  if (::connect(client_fd_, reinterpret_cast<sockaddr*>(&socket_config), static_cast<unsigned int>(path_length)) == -1)
  {
    std::cerr << "[scalopus] Could not connect socket." << std::endl;
    return false;
  }
  connections_.insert(client_fd_);

  running_ = true;
  thread_ = std::thread([&](){work();});

  return true;
}

std::shared_future<Data> TransportUnix::request(const std::string& remote_endpoint_name, const Data& outgoing)
{
  size_t request_id = request_counter_++;
  protocol::Msg outgoing_msg;
  outgoing_msg.endpoint = remote_endpoint_name;
  outgoing_msg.data = outgoing;
  outgoing_msg.request_id = request_id;

  auto promise = std::promise<Data>();
  auto shared_future = std::shared_future<Data>(promise.get_future());

  {
    std::lock_guard<std::mutex> lock(write_lock_);
    if (!protocol::send(client_fd_, outgoing_msg))
    {
      //  promise.set_exception(std::make_exception_ptr(communication_error("Failed to send data.")));
      throw communication_error("Failed to send data.");
      return shared_future;
    }
  }

  // Send succeeded, store the promise in the map, its result should be pending.
  std::lock_guard<std::mutex> lock(request_lock_);
  ongoing_requests_[{remote_endpoint_name, request_id}] = std::move(promise);
  return shared_future;
}

bool TransportUnix::isConnected() const
{
  return (client_fd_ != 0) || (server_fd_ != 0);
}

std::vector<std::size_t> TransportUnix::getProviders(const std::string& suffix)
{
  std::ifstream infile("/proc/net/unix");
  std::vector<std::size_t> res;
  //  Num       RefCount Protocol Flags    Type St Inode Path
  //  0000000000000000: 00000002 00000000 00010000 0001 01 235190 @16121_scalopus
  std::string line;
  while (std::getline(infile, line))
  {
    if (line.size() < suffix.size())
    {
      continue;  // definitely is not a line we are interested in.
    }
    // std::basic_string::ends_with is c++20 :|
    if (line.substr(line.size() - suffix.size()) == suffix)
    {
      // We got a hit, extract the process id.
      const auto space_before_path = line.rfind(" ");
      const auto path = line.substr(space_before_path + 2);  // + 2 for space and @ symbol.
      const auto space_before_inode = line.rfind(" ", space_before_path - 1);
      const auto inode_str = line.substr(space_before_inode, space_before_path - space_before_inode);
      if (std::atoi(inode_str.c_str()) == 0)
      {
        // clients to the socket get this 0 inode address...
        continue;
      }
      char* tmp;
      res.emplace_back(std::strtoul(path.substr(0, path.size() - suffix.size()).c_str(), &tmp, 10));
    }
  }

  return res;
}

TransportUnix::~TransportUnix()
{
  // First, stop the thread
  running_ = false;
  thread_.join();

  // Then clean up all the connections.
  for (const auto& connection : connections_)
  {
    ::close(connection);
    ::shutdown(connection, 2);
  }
}

void TransportUnix::work()
{
  fd_set read_fds;
  fd_set write_fds;
  fd_set except_fds;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 10000;

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
    int select_result = select(nfds, &read_fds, &write_fds, &except_fds, &tv);

    if (select_result == -1)
    {
      std::cerr << "[scalopus]: Failure occured on select." << std::endl;
    }

    // Handle server stuff, accept new connection:
    if (FD_ISSET(server_fd_, &read_fds))
    {
      int client{ 0 };
      client = accept(server_fd_, nullptr, nullptr);
      if (client == -1)
      {
        std::cerr << "[scalopus] Could not accept client." << std::endl;
        continue;
      }
      connections_.insert(client);
    }

    // handle client readable:
    if (FD_ISSET(client_fd_, &read_fds))
    {
      protocol::Msg incoming;
      if (!protocol::receive(client_fd_, incoming))
      {
        // we failed reading data, if this happens we lost the connection.
        running_ = false;
        client_fd_ = 0;
      }

      // lock the requests map.
      std::lock_guard<std::mutex> lock(request_lock_);
      // Try to find a promise for the message we just received.
      auto request_it = ongoing_requests_.find({incoming.endpoint, incoming.request_id});
      if (request_it != ongoing_requests_.end())
      {
        request_it->second.set_value(incoming.data); // set the value into the promise.
        ongoing_requests_.erase(request_it);  // remove the request promise from the map.
      }
      else
      {
        // no active request for this outstanding. Hand it off to the endpoint with this name.
        std::lock_guard<std::mutex> elock(endpoint_mutex_);
        const auto it = endpoints_.find(incoming.endpoint);
        if (it != endpoints_.end())
        {
          protocol::Msg response;
          response.endpoint = incoming.endpoint;
          response.request_id = incoming.request_id;
          Data outgoing;
          if (it->second->unsolicited(*this, incoming.data, response.data))
          {
            std::lock_guard<std::mutex> wlock(write_lock_);
            protocol::send(client_fd_, response);
          }
        }
      }
    }

    if (FD_ISSET(server_fd_, &except_fds))
    {
      std::cout << "[scalopus] Exception on server " << std::endl;
    }

    // else; check everything.
    const auto copy_of_connections = connections_;
    for (const auto& connection : copy_of_connections)
    {
      if ((connection == server_fd_) || (connection == client_fd_))
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
            std::lock_guard<std::mutex> wlock(write_lock_);
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

    while(haveBroadcast())
    {
      const auto name_payload = popBroadcast();
      protocol::Msg broadcast;
      broadcast.endpoint = name_payload.first;
      broadcast.data = name_payload.second;
      for (const auto& connection : connections_)
      {
        if (connection == server_fd_)
        {
          continue;
        }
        std::lock_guard<std::mutex> wlock(write_lock_);
        protocol::send(connection, broadcast);
      }
    }
  }
}

bool TransportUnix::processMsg(const protocol::Msg& request, protocol::Msg& response)
{
  response.endpoint = request.endpoint;
  response.request_id = request.request_id;
  // Check if we have this endpoint.
  const auto it = endpoints_.find(request.endpoint);
  if (it != endpoints_.end())
  {
    Data outgoing;
    // Let the endpoint handle the data and if necessary respond.
    return it->second->handle(*this, request.data, response.data);
  }
  return false;
}

std::shared_ptr<Transport> transportServerUnix()
{
  auto t = std::make_unique<TransportUnix>();
  if (t->serve())
  {
    return t;
  }
  return nullptr;
}

std::shared_ptr<Transport> transportClientUnix(std::size_t pid)
{
  auto t = std::make_unique<TransportUnix>();
  if (t->connect(pid))
  {
    return t;
  }
  return nullptr;
}

std::vector<size_t> getTransportServersUnix()
{
  return TransportUnix::getProviders();
}

}  // namespace scalopus

