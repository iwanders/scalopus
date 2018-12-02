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
#include "protocol.h"

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <array>

namespace scalopus
{
namespace protocol
{

bool readData(int fd, size_t length, std::vector<char>& incoming)
{
  const size_t chunk_size = 4096;
  std::array<char, chunk_size> buf;
  incoming.resize(0);
  incoming.reserve(chunk_size);
  size_t received { 0 };
  while(true)
  {
    ssize_t chunk_received = ::read(fd, buf.data(), std::min(length - received, chunk_size));

    if (chunk_received > 0)
    {
      printf("[scalopus] read %zd bytes: %.*s\n", chunk_received, static_cast<int>(chunk_received), buf.data());
    }

    // Add the data to incoming.
    incoming.resize(received + static_cast<size_t>(chunk_received));
    std::memcpy(&incoming[received], buf.data(), static_cast<size_t>(chunk_received));
    received += static_cast<size_t>(chunk_received);

    if (received == length)
    {
      break;
    }

    if (chunk_received == chunk_size)
    {
      // read the next chunk.
      continue;
    }

    if (chunk_received == -1)
    {
      std::cerr << "[scalopus] Read error occured." << std::endl;
      return false;
    }
    else if (chunk_received == 0)
    {
      std::cerr << "[scalopus] EOF" << std::endl;
      return false;
    }
    std::cerr << "Total bytes from transmission.: " << received << std::endl;
    break;
  }
  return true;
}


bool receive(int fd, Msg& incoming)
{
  // Simple length-prefixed protocol:
  // uint16_t length_endpoint_name;
  // char[length_endpoint_name] endpoint_name;
  // uint32_t length_of_payload;
  // char[length_of_payload] payload;
  std::uint16_t length_endpoint_name { 0 };
  std::vector<char> tmp;
  if (readData(fd, sizeof(length_endpoint_name), tmp))
  {
    length_endpoint_name = *reinterpret_cast<decltype(length_endpoint_name)*>(tmp.data());
  }
  else
  {
    return false;
  }
  std::cout << "length_endpoint_name: " << length_endpoint_name << std::endl;

  std::string endpoint_name;
  if (readData(fd, length_endpoint_name, tmp))
  {
    incoming.endpoint.resize(length_endpoint_name);
    std::memcpy(&incoming.endpoint[0], tmp.data(), length_endpoint_name);
  }
  else
  {
    return false;
  }
  std::cout << "endpoint: " << incoming.endpoint << std::endl;

  std::uint32_t length_data { 0 };
  if (readData(fd, sizeof(length_data), tmp))
  {
    length_data = *reinterpret_cast<decltype(length_data)*>(tmp.data());
  }
  else
  {
    return false;
  }
  std::cout << "length length_data: " << length_data << std::endl;

  // Finally, read the data.
  if (readData(fd, length_data, incoming.data))
  {
    return true;
  }

  return false;
}

bool send(int fd, const Msg& outgoing)
{
  if (fd == 0)
  {
    return false;
  }
  uint16_t length_endpoint_name = static_cast<uint16_t>(outgoing.endpoint.size());
  uint32_t length_data = static_cast<uint32_t>(outgoing.data.size());

  // Send endpoint length
  if (write(fd, &length_endpoint_name, sizeof(length_endpoint_name)) != static_cast<ssize_t>(sizeof(length_endpoint_name)))
  {
    return false;
  }

  // Send endpoint name.
  if (write(fd, outgoing.endpoint.data(), length_endpoint_name) != static_cast<ssize_t>(length_endpoint_name))
  {
    return false;
  }

  // Send data length
  if (write(fd, &length_data, sizeof(length_data)) != static_cast<ssize_t>(sizeof(length_data)))
  {
    return false;
  }

  // Send data
  if (write(fd, outgoing.data.data(), outgoing.data.size()) != static_cast<ssize_t>(outgoing.data.size()))
  {
    return false;
  }
  return true;
}


}  // namespace protocol
}  // namespace scalopus
