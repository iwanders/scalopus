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
#ifndef SCALOPUS_INTERFACE_ENDPOINT_H
#define SCALOPUS_INTERFACE_ENDPOINT_H

#include <scalopus_interface/types.h>
#include <memory>
#include <string>
#include <vector>

namespace scalopus
{
class Transport;

/**
 * @brief The endpoint specifies the interface for by which the transport interacts with the endpoints.
 *        This specifies the interface for both the server side and the client side.
 */
class Endpoint
{
public:
  using Ptr = std::shared_ptr<Endpoint>;

  virtual std::string getName() const = 0;

  /**
   * @brief Let the endpoint handline incoming data.
   * @return True if outgoing should be returned over the transport. False if no outgoing message.
   */
  virtual bool handle(Transport& transport, const Data& incoming, Data& outgoing);

  /**
   * @brief Handle unsolicited data incoming over a client connection from the endpoint
   * In general this is to accept proactive / broadcast / publish style data.
   */
  virtual bool unsolicited(Transport& transport, const Data& incoming, Data& outgoing);

  /**
   * @brief Set the transport to be used by this endpoint. This is in general used by the client endpoint.
   */
  virtual void setTransport(const std::shared_ptr<Transport>& transport);

  virtual ~Endpoint() = default;
protected:
  std::weak_ptr<Transport> transport_;
};

}  // namespace scalopus
#endif  // SCALOPUS_INTERFACE_ENDPOINT_H
