/*
  Copyright (c) 2018-2019, Ivor Wanders
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of the author nor the names of contributors may be used to
    endorse or promote products derived from this software without specific
    prior written permission.

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

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <array>
#include <cstring>
#include <iostream>

namespace scalopus
{
namespace protocol
{
bool readData(int fd, size_t length, Data& incoming)
{
  // Technically this doesn't need to read in chunks anymore... since we always know the length.
  const size_t chunk_size = 4096;
  std::array<char, chunk_size> buf;
  incoming.resize(0);
  incoming.reserve(chunk_size);
  size_t received{ 0 };
  while (true)
  {
    ssize_t chunk_received = ::read(fd, buf.data(), std::min(length - received, chunk_size));
    if (chunk_received == -1)
    {
      return false;  // this should not happen as it indicates a broken transmission or broken file descriptor.
    }

    // Add the data to incoming.
    incoming.resize(received + static_cast<size_t>(chunk_received));
    std::memcpy(&incoming[received], buf.data(), static_cast<size_t>(chunk_received));
    received += static_cast<size_t>(chunk_received);

    // If we have received all data, continue.
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
      // an error occured.
      return false;
    }
    else if (chunk_received == 0)
    {
      // end of file.
      return false;
    }
    break;
  }
  return true;
}

bool receive(int fd, Msg& incoming)
{
  // Simple length prefixed protocol:

  // size_t request_id;
  // uint16_t length_endpoint_name;
  // char[length_endpoint_name] endpoint_name;
  // uint32_t length_of_payload;
  // char[length_of_payload] payload;

  Data tmp;

  // Read the request id.
  if (readData(fd, sizeof(incoming.request_id), tmp))
  {
    incoming.request_id = *reinterpret_cast<decltype(incoming.request_id)*>(tmp.data());
  }
  else
  {
    return false;
  }

  std::uint16_t length_endpoint_name{ 0 };
  if (readData(fd, sizeof(length_endpoint_name), tmp))
  {
    length_endpoint_name = *reinterpret_cast<decltype(length_endpoint_name)*>(tmp.data());
  }
  else
  {
    // We also hit this if the socket can be closed...
    return false;
  }

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

  std::uint32_t length_data{ 0 };
  if (readData(fd, sizeof(length_data), tmp))
  {
    length_data = *reinterpret_cast<decltype(length_data)*>(tmp.data());
  }
  else
  {
    return false;
  }

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

  // Send request id.
  if (::send(fd, &outgoing.request_id, sizeof(outgoing.request_id), MSG_NOSIGNAL) !=
      static_cast<ssize_t>(sizeof(outgoing.request_id)))
  {
    return false;
  }

  // Send endpoint length
  if (::send(fd, &length_endpoint_name, sizeof(length_endpoint_name), MSG_NOSIGNAL) !=
      static_cast<ssize_t>(sizeof(length_endpoint_name)))
  {
    return false;
  }

  // Send endpoint name.
  if (::send(fd, outgoing.endpoint.data(), length_endpoint_name, MSG_NOSIGNAL) !=
      static_cast<ssize_t>(length_endpoint_name))
  {
    return false;
  }

  // Send data length
  if (::send(fd, &length_data, sizeof(length_data), MSG_NOSIGNAL) != static_cast<ssize_t>(sizeof(length_data)))
  {
    return false;
  }

  // Send data
  if (::send(fd, outgoing.data.data(), outgoing.data.size(), MSG_NOSIGNAL) !=
      static_cast<ssize_t>(outgoing.data.size()))
  {
    return false;
  }
  return true;
}

}  // namespace protocol
}  // namespace scalopus
