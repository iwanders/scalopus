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
#include "scalopus_interface.h"
#include <scalopus_interface/endpoint_manager.h>
#include <scalopus_interface/trace_event_provider.h>
#include <scalopus_interface/trace_event_source.h>


namespace scalopus
{
namespace py = pybind11;

std::string PyEndpoint::getName() const
{
  PYBIND11_OVERLOAD_PURE(std::string, PyEndpoint, getName, );
}

bool PyEndpoint::handle(Transport& transport, const Data& incoming, Data& outgoing)
{
  PYBIND11_OVERLOAD(bool, Endpoint, handle, transport, incoming, outgoing);
}

bool PyEndpoint::unsolicited(Transport& transport, const Data& incoming, Data& outgoing)
{
  PYBIND11_OVERLOAD(bool, Endpoint, unsolicited, transport, incoming, outgoing);
}

void add_scalopus_interface(py::module& m)
{
  py::class_<Destination, Destination::Ptr> destination(m, "Destination");
  destination.def("__str__", &Destination::operator std::string);
  destination.def("hash_code", &Destination::hash_code);

  py::class_<Transport, Transport::Ptr> transport_interface(m, "Transport");
  transport_interface.def("addEndpoint", &Transport::addEndpoint);
  transport_interface.def("isConnected", &Transport::isConnected);

  py::class_<Endpoint, Endpoint::Ptr> endpoint(m, "Endpoint");
  endpoint.def("getName", &Endpoint::getName);

  py::class_<PyEndpoint, PyEndpoint::Ptr, Endpoint> py_endpoint(m, "PyEndpoint");
  py_endpoint.def(py::init<>());
  py_endpoint.def("getName", &PyEndpoint::getName);
  py_endpoint.def("handle", &PyEndpoint::handle);
  py_endpoint.def("unsolicited", &PyEndpoint::unsolicited);

  py::class_<TraceEventProvider, TraceEventProvider::Ptr> trace_event_provider(m, "TraceEventProvider");
  trace_event_provider.def("makeSource", &TraceEventProvider::makeSource);

  py::class_<TraceEventSource, TraceEventSource::Ptr> trace_event_source(m, "TraceEventSource");
  trace_event_source.def("startInterval", &TraceEventSource::startInterval);
  trace_event_source.def("stopInterval", &TraceEventSource::stopInterval);
  trace_event_source.def("work", &TraceEventSource::work);
  //  trace_event_source.def("finishInterval", &TraceEventSource::finishInterval);

  py::class_<EndpointManager, EndpointManager::Ptr> endpoint_manager(m, "EndpointManager");
  endpoint_manager.def("endpoints", &EndpointManager::endpoints);
  //  endpoint_manager.def("addEndpointFactory", &EndpointManager::addEndpointFactory);

  py::class_<TransportFactory, TransportFactory::Ptr> transport_factory(m, "TransportFactory");
  transport_factory.def("discover", &TransportFactory::discover);
  transport_factory.def("serve", &TransportFactory::serve);
  transport_factory.def("connect", &TransportFactory::connect);

}

}  // namespace scalopus
