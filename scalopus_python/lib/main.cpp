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
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <scalopus_general/general.h>
#include <scalopus_interface/endpoint_manager.h>
#include <scalopus_interface/trace_event_provider.h>
#include <scalopus_interface/trace_event_source.h>
#include <scalopus_tracing/native_tracepoint.h>
#include <scalopus_tracing/tracing.h>
#include <scalopus_transport/transport_mock.h>
#include <scalopus_transport/transport_unix.h>

#ifdef SCALOPUS_TRACING_HAVE_LTTNG
#include <scalopus_tracing/lttng_tracepoint.h>
#endif

namespace py = pybind11;

namespace scalopus
{
class PyEndpoint : public Endpoint
{
public:
  using Ptr = std::shared_ptr<PyEndpoint>;
  using Endpoint::Endpoint;  // Inherit constructors

  std::string getName() const override
  {
    PYBIND11_OVERLOAD_PURE(std::string, PyEndpoint, getName, );
  }

  bool handle(Transport& transport, const Data& incoming, Data& outgoing) override
  {
    PYBIND11_OVERLOAD(bool, Endpoint, handle, transport, incoming, outgoing);
  }
};

}  // namespace scalopus

PYBIND11_MODULE(scalopus_python_lib, m)
{
  // scalopus_interface
  py::class_<scalopus::Destination, scalopus::Destination::Ptr> destination(m, "Destination");
  destination.def("__str__", &scalopus::Destination::operator std::string);
  destination.def("hash_code", &scalopus::Destination::hash_code);

  py::class_<scalopus::Transport, scalopus::Transport::Ptr> transport_interface(m, "Transport");
  transport_interface.def("addEndpoint", &scalopus::Transport::addEndpoint);
  transport_interface.def("isConnected", &scalopus::Transport::isConnected);

  py::class_<scalopus::Endpoint, scalopus::Endpoint::Ptr> endpoint(m, "Endpoint");
  endpoint.def("getName", &scalopus::Endpoint::getName);

  py::class_<scalopus::PyEndpoint, scalopus::PyEndpoint::Ptr> py_endpoint(m, "PyEndpoint", endpoint);
  py_endpoint.def(py::init<>());
  py_endpoint.def("getName", &scalopus::PyEndpoint::getName);

  py::class_<scalopus::TraceEventProvider, scalopus::TraceEventProvider::Ptr> trace_event_provider(m, "TraceEventProvid"
                                                                                                      "er");
  trace_event_provider.def("makeSource", &scalopus::TraceEventProvider::makeSource);

  py::class_<scalopus::TraceEventSource, scalopus::TraceEventSource::Ptr> trace_event_source(m, "TraceEventSource");
  trace_event_source.def("startInterval", &scalopus::TraceEventSource::startInterval);
  trace_event_source.def("stopInterval", &scalopus::TraceEventSource::stopInterval);
  trace_event_source.def("work", &scalopus::TraceEventSource::work);
  //  trace_event_source.def("finishInterval", &scalopus::TraceEventSource::finishInterval);

  py::class_<scalopus::EndpointManager, scalopus::EndpointManager::Ptr> endpoint_manager(m, "EndpointManager");
  endpoint_manager.def("endpoints", &scalopus::EndpointManager::endpoints);
  //  endpoint_manager.def("addEndpointFactory", &scalopus::EndpointManager::addEndpointFactory);

  py::class_<scalopus::TransportFactory, scalopus::TransportFactory::Ptr> transport_factory(m, "TransportFactory");
  transport_factory.def("discover", &scalopus::TransportFactory::discover);
  transport_factory.def("serve", &scalopus::TransportFactory::serve);
  transport_factory.def("connect", &scalopus::TransportFactory::connect);

  // scalopus_transport
  py::module transport = m.def_submodule("transport", "The transport related components.");
  py::class_<scalopus::TransportUnixFactory, scalopus::TransportUnixFactory::Ptr> transport_factory_unix(
      transport, "TransportUnixFactory", transport_factory);
  transport_factory_unix.def(py::init<>());
  py::class_<scalopus::TransportMockFactory, scalopus::TransportMockFactory::Ptr> transport_factory_mock(
      transport, "TransportMockFactory", transport_factory);
  transport_factory_mock.def(py::init<>());

  // scalopus_general
  py::module general = m.def_submodule("general", "The general components.");
  py::class_<scalopus::EndpointIntrospect, scalopus::EndpointIntrospect::Ptr> endpoint_introspect(
      general, "EndpointIntrospect", endpoint);
  endpoint_introspect.def(py::init<>());
  endpoint_introspect.def("supported", &scalopus::EndpointIntrospect::supported);

  py::class_<scalopus::EndpointProcessInfo::ProcessInfo> endpoint_process_info_info(general, "ProcessInfo");
  endpoint_process_info_info.def_readwrite("name", &scalopus::EndpointProcessInfo::ProcessInfo::name);
  endpoint_process_info_info.def_readwrite("threads", &scalopus::EndpointProcessInfo::ProcessInfo::threads);

  py::class_<scalopus::EndpointProcessInfo, scalopus::EndpointProcessInfo::Ptr> endpoint_process_info(
      general, "EndpointProcessInfo", endpoint);
  endpoint_process_info.def(py::init<>());
  endpoint_process_info.def("setProcessName", &scalopus::EndpointProcessInfo::setProcessName);
  endpoint_process_info.def("processInfo", &scalopus::EndpointProcessInfo::processInfo);

  general.def("setThreadName",
              [](const std::string& name) { scalopus::ThreadNameTracker::getInstance().setCurrentName(name); });

  // scalopus_tracing
  py::module tracing = m.def_submodule("tracing", "The tracing specific components.");
  py::class_<scalopus::EndpointTraceMapping> endpoint_trace_mapping(tracing, "EndpointTraceMapping", endpoint);
  endpoint_trace_mapping.def(py::init<>());
  endpoint_trace_mapping.def("mapping", &scalopus::EndpointTraceMapping::mapping);

  py::module native = tracing.def_submodule("native", "The native specific components.");
  py::class_<scalopus::EndpointNativeTraceSender> endpoint_native_trace_sender(native, "EndpointNativeTraceSender",
                                                                               endpoint);
  endpoint_native_trace_sender.def(py::init<>());

  tracing.def("setTraceName", [](const unsigned int id, const std::string& name) {
    scalopus::ScopeTraceTracker::getInstance().insert(id, name);
  });

#ifdef SCALOPUS_TRACING_HAVE_LTTNG
  py::module lttng = tracing.def_submodule("lttng", "The lttng specific components.");
  lttng.def("scope_entry", &scalopus::lttng::scope_entry);
  lttng.def("scope_exit", &scalopus::lttng::scope_exit);
#endif

  native.def("scope_entry", &scalopus::native::scope_entry);
  native.def("scope_exit", &scalopus::native::scope_exit);
}
