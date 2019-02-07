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
#include "json_util.h"
#include "pybind_fix.h"


namespace scalopus
{
namespace py = pybind11;

bool pyToData(const py::object& obj, Data& outgoing)
{
  // to string, and then from string to bytes.
  if (py::isinstance<py::str>(obj))
  {
    std::string my_result = obj.cast<std::string>();
    outgoing = Data{ my_result.begin(), my_result.end() };
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
  return py::bytes{ std::string{ data.begin(), data.end() } };
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
      py::bytes my_bytes{ std::string{ incoming.begin(), incoming.end() } };
      auto obj = overload(transport, my_bytes);
      return pyToData(obj, outgoing);
    }
  }
  return Endpoint::handle(transport, incoming, outgoing);
}

bool PyEndpoint::unsolicited(Transport& transport, const Data& incoming, Data& outgoing)
{
  {
    pybind11::gil_scoped_acquire gil;
    pybind11::function overload = pybind11::get_overload(this, "unsolicited");
    if (overload)
    {
      // from data to string and then to bytes.
      py::bytes my_bytes{ std::string{ incoming.begin(), incoming.end() } };
      auto obj = overload(transport, my_bytes);
      return pyToData(obj, outgoing);
    }
  }
  return Endpoint::unsolicited(transport, incoming, outgoing);
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

TraceEventSource::Ptr PyTraceEventProvider::makeSource()
{
  std::cout << "PyTraceEventProvider: " << this << std::endl;
  PYBIND11_OVERLOAD_PURE(TraceEventSource::Ptr, TraceEventProvider, makeSource, );
}

void PyTraceEventSource::startInterval()
{
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  PYBIND11_OVERLOAD(void, TraceEventSource, startInterval, );
}

void PyTraceEventSource::stopInterval()
{
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  {

    pybind11::gil_scoped_acquire gil;
    pybind11::function overload = pybind11::get_overload(this, "finishInterval");
    std::cout << "overload: " << bool{overload} << std::endl;
    std::cout << "this: " << this << std::endl;
  }
  PYBIND11_OVERLOAD(void, TraceEventSource, stopInterval, );
}

std::vector<json> PyTraceEventSource::finishInterval()
{
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  // The body here should be:
  // PYBIND11_OVERLOAD(std::vector<json>, TraceEventSource, finishInterval,);
  // but that causes a incomplete type on resolving the 'is_copy_constructable' template.
  // So we do it by hand here.

  pybind11::gil_scoped_acquire gil;
  pybind11::function overload = pybind11::get_overload(this, "finishInterval");
  if (overload)
  {
    auto obj = overload();
    json returned_value = obj;
    return returned_value.get<std::vector<json>>();
  }
  return {};
}

// Maybe convert to bytes using https://pybind11.readthedocs.io/en/master/advanced/cast/stl.html ?
void add_scalopus_interface(py::module& m)
{
  py::module interface = m.def_submodule("interface", "The interface components.");
  py::class_<Destination, Destination::Ptr> destination(interface, "Destination");
  destination.def("__str__", &Destination::operator std::string);
  destination.def("hash_code", &Destination::hash_code);

  py::class_<PendingResponse, PendingResponse::Ptr> pending_response(interface, "PendingResponse");
  pending_response.def("wait_for", &PendingResponse::wait_for);

  py::class_<Transport, Transport::Ptr> transport_interface(interface, "Transport");
  transport_interface.def("addEndpoint", &Transport::addEndpoint, py::keep_alive<1, 2>());
  transport_interface.def("isConnected", &Transport::isConnected);
  transport_interface.def("broadcast", &Transport::broadcast);
  transport_interface.def("request", [](Transport& transport, const std::string& name, const py::object& outgoing) {
    return std::make_shared<PendingResponse>(transport.request(name, pyToData(outgoing)));
  });

  py::class_<Endpoint, PyEndpoint, Endpoint::Ptr> py_endpoint(interface, "Endpoint");
  py_endpoint.def(py::init<>());
  py_endpoint.def("getName", &Endpoint::getName);
  py_endpoint.def("handle", &Endpoint::handle);
  py_endpoint.def("unsolicited", &Endpoint::unsolicited);
  py_endpoint.def("getTransport", &Endpoint::getTransport, py::return_value_policy::reference);

  py::class_<TraceEventProvider, PyTraceEventProvider, TraceEventProvider::Ptr> trace_event_provider(
      interface, "TraceEventProvider");
  trace_event_provider.def(py::init<>());
  trace_event_provider.def("makeSource", &TraceEventProvider::makeSource);

  py::class_<TraceEventSource, PyTraceEventSource, TraceEventSource::Ptr> trace_event_source(interface,
                                                                                             "TraceEventSource");
  trace_event_source.def(py::init<>());
  trace_event_source.def("startInterval", &TraceEventSource::startInterval);
  trace_event_source.def("stopInterval", &TraceEventSource::stopInterval);
  //  trace_event_source.def("work", &TraceEventSource::work);
  trace_event_source.def("finishInterval", &TraceEventSource::finishInterval);  // implicit conversion from json =)

  py::class_<EndpointManager, EndpointManager::Ptr> endpoint_manager(interface, "EndpointManager");
  endpoint_manager.def("endpoints", &EndpointManager::endpoints, py::return_value_policy::copy);
  endpoint_manager.def("addEndpointFactory", [](EndpointManager& manager, std::string name, py::object fun) {
    manager.addEndpointFactory(name, [fun](const Transport::Ptr& transport) {
      pybind11::gil_scoped_acquire gil;
      py::object result_py = fun(transport);
      if (py::isinstance<py::none>(result_py))
      {
        // python side returned none... provider went weak.
        return Endpoint::Ptr{};
      }
      Endpoint::Ptr endpoint = result_py.cast<Endpoint::Ptr>();
      if (endpoint == nullptr)
      {
        throw py::value_error("Could not cast returned object to Endpoint.");
      }
      return endpoint;
    });
  });

  py::class_<TransportFactory, TransportFactory::Ptr> transport_factory(interface, "TransportFactory");
  transport_factory.def("discover", &TransportFactory::discover);
  transport_factory.def("serve", &TransportFactory::serve);
  transport_factory.def("connect", &TransportFactory::connect, py::return_value_policy::copy);
}

}  // namespace scalopus
