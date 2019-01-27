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
#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <scalopus_interface/endpoint_manager.h>

namespace scalopus
{
namespace py = pybind11;

class PyEndpoint : public Endpoint
{
public:
  using Ptr = std::shared_ptr<PyEndpoint>;
  using Endpoint::Endpoint;  // Inherit constructors

  std::string getName() const override;
  bool handle(Transport& transport, const Data& incoming, Data& outgoing) override;
  bool unsolicited(Transport& transport, const Data& incoming, Data& outgoing) override;
};

class PendingResponse
{
public:
  using Ptr = std::shared_ptr<PendingResponse>;
  PendingResponse(Transport::PendingResponse resp);
  py::object wait_for(int milliseconds);

  Transport::PendingResponse resp_;
};

void add_scalopus_interface(py::module&);
}  // namespace scalopus
