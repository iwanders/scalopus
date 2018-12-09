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
#include "transport_client_unix.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include "protocol.h"

namespace scalopus
{
TransportClientUnix::TransportClientUnix()
{
}

TransportClientUnix::~TransportClientUnix()
{
  disconnect();
}

void TransportClientUnix::disconnect()
{
  running_ = false;
  incoming_handler_.join();
  if (fd_)
  {
    ::close(fd_);
    ::shutdown(fd_, 2);
  }
}

bool TransportClientUnix::connect(std::size_t pid, const std::string& suffix)
{
  fd_ = socket(AF_UNIX, SOCK_STREAM, 0);

  if (fd_ == -1)
  {
    std::cerr << "[scalopus] Could not create socket." << std::endl;
    // Should we continue if this happens?
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
  if (::connect(fd_, reinterpret_cast<sockaddr*>(&socket_config), static_cast<unsigned int>(path_length)) == -1)
  {
    std::cerr << "[scalopus] Could not connect socket." << std::endl;
    return false;
  }

  running_ = true;
  incoming_handler_ = std::thread([&](){work();});

  return true;
}

std::shared_future<Data> TransportClientUnix::request(const std::string& remote_endpoint_name, const Data& outgoing, size_t request_id)
{
  if (request_id == 0)
  {
    request_id = request_counter_++;
  }
  protocol::Msg outgoing_msg;
  outgoing_msg.endpoint = remote_endpoint_name;
  outgoing_msg.data = outgoing;
  outgoing_msg.request_id = request_id;

  auto promise = std::promise<Data>();
  auto shared_future = std::shared_future<Data>(promise.get_future());

  {
    std::lock_guard<std::mutex> lock(write_lock_);
    if (!protocol::send(fd_, outgoing_msg))
    {
      promise.set_exception(std::make_exception_ptr(communication_error("Failed to send data.")));
      return shared_future;
    }
  }

  // Send succeeded, store the promise in the map, its result should be pending.
  std::lock_guard<std::mutex> lock(request_lock_);
  ongoing_requests_[{remote_endpoint_name, request_id}] = std::move(promise);
  return shared_future;
}

bool TransportClientUnix::isConnected() const
{
  return fd_ != 0;
}

std::vector<std::size_t> TransportClientUnix::getProviders(const std::string& suffix)
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

void TransportClientUnix::work()
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
    FD_SET(fd_, &read_fds);

    int select_result = select(fd_ + 1, &read_fds, &write_fds, &except_fds, &tv);

    if (select_result == -1)
    {
      std::cerr << "[scalopus]: Failure occured on select." << std::endl;
    }

    // We can read...
    if (FD_ISSET(fd_, &read_fds))
    {
      protocol::Msg incoming;
      if (!protocol::receive(fd_, incoming))
      {
        // we failed reading data, if this happens we lost the connection.
        running_ = false;
        fd_ = 0;
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
    }
  }
}

std::shared_ptr<TransportClient> transportClientUnix(const size_t pid)
{
  auto z = std::make_shared<TransportClientUnix>();
  z->connect(pid);
  return z;
}
std::vector<size_t> getTransportServersUnix()
{
  return TransportClientUnix::getProviders();
}

}  // namespace scalopus
