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
#include "protocol.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <iostream>
#include <fstream>

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

  return true;
}

bool TransportClientUnix::send(const std::string& remote_endpoint_name, const std::vector<char>& data, std::vector<char>& response)
{
  protocol::Msg outgoing;
  outgoing.endpoint = remote_endpoint_name;
  outgoing.data = data;
  if (!protocol::send(fd_, outgoing))
  {
    std::cout << "Send fail" << std::endl;
    return false;
  }
  protocol::Msg incoming;
  if (!protocol::receive(fd_, incoming))
  {
    std::cerr << "receive fail" << std::endl;
    return false;
  }

  response = incoming.data;
  return true;
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

}


