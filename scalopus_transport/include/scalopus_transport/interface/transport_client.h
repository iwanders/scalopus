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

#ifndef SCALOPUS_INTERFACE_TRANSPORT_CLIENT_H
#define SCALOPUS_INTERFACE_TRANSPORT_CLIENT_H

#include <scalopus_transport/interface/client.h>
#include <scalopus_transport/interface/types.h>
#include <memory>
#include <map>
#include <future>

namespace scalopus
{

class TransportClient
{
public:
  using WeakPtr = std::weak_ptr<TransportClient>;
  using Ptr = std::shared_ptr<TransportClient>;

  /**
   * @brief Prepare (and send) a request.
   * @param remote_endpoint_name The endpoint to which this request should be sent.
   * @param outgoing The data that should be sent to the endpoint. 
   * @param request_id The request id, if this is zero this is automatically incremented by the client. Otherwise it is
   *        used to get the future for endpoints that send by themselves without a request.
   */
  virtual std::shared_future<Data> request(const std::string& remote_endpoint_name, const Data& outgoing, size_t request_id = 0) = 0;

  virtual ~TransportClient();
  virtual bool isConnected() const = 0;
};


}  // namespace scalopus
#endif  // SCALOPUS_INTERFACE_TRANSPORT_CLIENT_H