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
#ifndef SCALOPUS_PROTOCOL_H
#define SCALOPUS_PROTOCOL_H
#include <string>
#include <vector>
#include "scalopus_transport/interface/types.h"

namespace scalopus
{
/**
 * @brief Simple fixed length protocol for use in the unix domain socket transport. The format on the wire is:
 * request_id | endpoint_name_length | endpoint_name | data_length | data.
 */
namespace protocol
{
struct Msg
{
  size_t request_id{ 0 };  //!< The request id associated to this request.
  std::string endpoint;    //!< Endpoint name for this message.
  Data data;               //!< Data in this message.
};

/**
 * @brief Read fixed length data from a socket and place it in incoming.
 * @return True if success, false if error occured and the socket can be closed.
 */
bool readData(int fd, size_t length, Data& incoming);

/**
 * @brief Receive a message from a socket.
 * @return True if success, false if error occured and the socket can be closed.
 */
bool receive(int fd, Msg& incoming);

/**
 * @brief Send a message from a socket.
 * @return True if success, false if error occured and the socket can be closed.
 */
bool send(int fd, const Msg& send);
}  // namespace protocol

}  // namespace scalopus

#endif
