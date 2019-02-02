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
#include <pybind11/chrono.h>
#include <scalopus_interface/endpoint_manager.h>
#include <scalopus_interface/trace_event_provider.h>
#include <scalopus_interface/trace_event_source.h>

namespace scalopus
{
namespace py = pybind11;


bool pyToData(const py::object& obj, Data& outgoing)
{
  // to string, and then from string to bytes.
  if (py::isinstance<py::str>(obj))
  {
    std::string my_result = obj.cast<std::string>();
    outgoing = Data{my_result.begin(), my_result.end()};
    return true;
  }
  // also try buffer, which is bytearray and bytes.
  if (py::isinstance<py::buffer>(obj))
  {
    outgoing = obj.cast<Data>();
    return true;
  }
  // If it is a list, it's probably a list of integers
  if (py::isinstance<py::list>(obj))
  {
    outgoing = obj.cast<Data>();
    return true;
  }
  return false;
}

Data pyToData(const py::object& obj)
{
  Data res;
  if (!pyToData(obj, res))
  {
    throw py::value_error("Cannot convert provided object to Data");
  }
  return res;
}

py::object dataToPy(const Data& data)
{
  return py::bytes{std::string{data.begin(), data.end()}};
}

std::string PyEndpoint::getName() const
{
  PYBIND11_OVERLOAD_PURE(std::string, Endpoint, getName, );
}

bool PyEndpoint::handle(Transport& transport, const Data& incoming, Data& outgoing)
{
  {
    pybind11::gil_scoped_acquire gil;
    pybind11::function overload = pybind11::get_overload(this, "handle");
    if (overload)
    {
      // from data to string and then to bytes.
      py::bytes my_bytes{std::string{incoming.begin(), incoming.end()}};
      auto obj = overload(transport, my_bytes);
      return pyToData(obj, outgoing);
    }
  }
  return Endpoint::handle(transport, incoming, outgoing);
}

bool PyEndpoint::unsolicited(Transport& transport, const Data& incoming, Data& outgoing)
{
  PYBIND11_OVERLOAD(bool, Endpoint, unsolicited, transport, incoming, outgoing);
}

void PyEndpoint::setTransport(Transport* transport)
{
  PYBIND11_OVERLOAD(void, Endpoint, setTransport, transport);
}

PendingResponse::PendingResponse(Transport::PendingResponse resp) : resp_{ resp }
{
}

py::object PendingResponse::wait_for(double seconds)
{
  std::cout << "Blocking for " << seconds << " s on future" << std::endl;

  py::gil_scoped_release release;
  if (resp_->wait_for(std::chrono::duration<double>(seconds)) == std::future_status::ready)
  {
    std::cout << "Succesfully waited for future" << std::endl;
    return dataToPy(resp_->get());
  }
  std::cout << "bummer " << std::endl;
  return py::cast<py::none>(Py_None);
}

// Maybe convert to bytes using https://pybind11.readthedocs.io/en/master/advanced/cast/stl.html ?
void add_scalopus_interface(py::module& m)
{
  py::class_<Destination, Destination::Ptr> destination(m, "Destination");
  destination.def("__str__", &Destination::operator std::string);
  destination.def("hash_code", &Destination::hash_code);

  py::class_<PendingResponse, PendingResponse::Ptr> pending_response(m, "PendingResponse");
  pending_response.def("wait_for", &PendingResponse::wait_for);

  py::class_<Transport, Transport::Ptr> transport_interface(m, "Transport");
  transport_interface.def("addEndpoint", &Transport::addEndpoint);
  transport_interface.def("isConnected", &Transport::isConnected);
  transport_interface.def("broadcast", &Transport::broadcast);
  transport_interface.def("request", [](Transport& transport, const std::string& name, const py::object& outgoing) {
    return std::make_shared<PendingResponse>(transport.request(name, pyToData(outgoing)));
  });

  //  py::class_<Endpoint, Endpoint::Ptr> endpoint(m, "Endpoint");
  //  endpoint.def("getName", &Endpoint::getName);
  //  endpoint.def("handle", &Endpoint::handle);
  //  endpoint.def("unsolicited", &Endpoint::unsolicited);

  py::class_<Endpoint, PyEndpoint, Endpoint::Ptr> py_endpoint(m, "Endpoint");
  py_endpoint.def(py::init<>());
  py_endpoint.def("getName", &Endpoint::getName);
  py_endpoint.def("handle", &Endpoint::handle);
  py_endpoint.def("unsolicited", &Endpoint::unsolicited);
  py_endpoint.def("getTransport", &Endpoint::getTransport);

  py::class_<TraceEventProvider, TraceEventProvider::Ptr> trace_event_provider(m, "TraceEventProvider");
  trace_event_provider.def("makeSource", &TraceEventProvider::makeSource);

  py::class_<TraceEventSource, TraceEventSource::Ptr> trace_event_source(m, "TraceEventSource");
  trace_event_source.def("startInterval", &TraceEventSource::startInterval);
  trace_event_source.def("stopInterval", &TraceEventSource::stopInterval);
  trace_event_source.def("work", &TraceEventSource::work);
  //  trace_event_source.def("finishInterval", &TraceEventSource::finishInterval);
  trace_event_source.def("finishInterval", [](TraceEventSource& source)
  {
    auto res = source.finishInterval();
    std::vector<std::string> python_res;
    for (const auto events : res)
    {
      python_res.push_back(events.dump());
    }
    return python_res;
  });

  py::class_<EndpointManager, EndpointManager::Ptr> endpoint_manager(m, "EndpointManager");
  endpoint_manager.def("endpoints", &EndpointManager::endpoints);
  //  endpoint_manager.def("addEndpointFactory", &EndpointManager::addEndpointFactory);

  py::class_<TransportFactory, TransportFactory::Ptr> transport_factory(m, "TransportFactory");
  transport_factory.def("discover", &TransportFactory::discover);
  transport_factory.def("serve", &TransportFactory::serve);
  transport_factory.def("connect", &TransportFactory::connect);
}

}  // namespace scalopus
