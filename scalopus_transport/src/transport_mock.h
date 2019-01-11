/*
  Copyright (c) 2019, Ivor Wanders
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

#ifndef SCALOPUS_TRANSPORT_TRANSPORT_MOCK_INTERNAL_H
#define SCALOPUS_TRANSPORT_TRANSPORT_MOCK_INTERNAL_H

#include <scalopus_interface/transport_factory.h>
#include <scalopus_transport/transport_mock.h>
#include <future>
#include <map>
#include <set>
#include <thread>
#include <memory>
#include <utility>
#include <vector>
#include "protocol.h"

namespace scalopus
{
/**
 * @brief A transport for testing basically, a server is created to which clients can be attached.
 */
class TransportMock : public Transport, public std::enable_shared_from_this<TransportMock>
{
public:
  using Ptr = std::shared_ptr<TransportMock>;
  TransportMock();
  TransportMock(Ptr server);  // Create a mock transport client side connected to the server.

  // From Transport superclass.
  PendingResponse request(const std::string& remote_endpoint_name, const Data& outgoing);
  void broadcast(const std::string& remote_endpoint_name, const Data& outgoing);

  std::size_t pendingRequests() const;

  void addClient(Transport::WeakPtr client);
  bool isConnected() const;
private:
  Ptr server_;
  std::vector<Transport::WeakPtr> clients_;
};

}  // namespace scalopus
#endif  // SCALOPUS_TRANSPORT_TRANSPORT_MOCK_INTERNAL_H