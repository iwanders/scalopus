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
#include "scalopus_tracing.h"

#include <pybind11/stl.h>
#include <scalopus_tracing/native_tracepoint.h>
#include <scalopus_tracing/tracing.h>

#ifdef SCALOPUS_TRACING_HAVE_LTTNG
#include <scalopus_tracing/lttng_provider.h>
#include <scalopus_tracing/lttng_tracepoint.h>
#endif

#include <scalopus_tracing/nop_tracepoint.h>

#include <scalopus_tracing/native_trace_provider.h>
#include <scalopus_tracing/trace_configurator.h>

static std::size_t uniqueTraceId()
{
  static std::atomic_size_t counter{ 0 };
  return counter++;
}

namespace scalopus
{
namespace py = pybind11;
void add_scalopus_tracing(py::module& m)
{
  py::module tracing = m.def_submodule("tracing", "The tracing specific components.");
  tracing.def("uniqueTraceId", &uniqueTraceId);

  py::enum_<MarkLevel>(tracing, "MarkLevel")
      .value("GLOBAL", MarkLevel::GLOBAL)
      .value("PROCESS", MarkLevel::PROCESS)
      .value("THREAD", MarkLevel::THREAD);

  py::class_<EndpointTraceMapping, EndpointTraceMapping::Ptr, Endpoint> endpoint_trace_mapping(tracing,
                                                                                               "EndpointTraceMapping");
  endpoint_trace_mapping.def(py::init<>());
  endpoint_trace_mapping.def("mapping", &EndpointTraceMapping::mapping);
  endpoint_trace_mapping.def_static("factory", &EndpointTraceMapping::factory);
  endpoint_trace_mapping.def_property_readonly_static("name",
                                                      [](py::object /* self */) { return EndpointTraceMapping::name; });

  py::module native = tracing.def_submodule("native", "The native specific components.");
  py::class_<EndpointNativeTraceSender, EndpointNativeTraceSender::Ptr, Endpoint> endpoint_native_trace_sender(
      native, "EndpointNativeTraceSender");
  endpoint_native_trace_sender.def_property_readonly_static(
      "name", [](py::object /* self */) { return EndpointNativeTraceSender::name; });
  endpoint_native_trace_sender.def(py::init<>());

  tracing.def("setTraceName", [](const unsigned int id, const std::string& name) {
    StaticStringTracker::getInstance().insert(id, name);
  });

  tracing.def("getThreadState", []() {
    TraceConfigurator& configurator = TraceConfigurator::getInstance();
    return configurator.getThreadState();
  });
  tracing.def("setThreadState", [](bool new_state) {
    TraceConfigurator& configurator = TraceConfigurator::getInstance();
    return configurator.setThreadState(new_state);
  });
  tracing.def("getProcessState", []() {
    TraceConfigurator& configurator = TraceConfigurator::getInstance();
    return configurator.getProcessState();
  });
  tracing.def("setProcessState", [](bool new_state) {
    TraceConfigurator& configurator = TraceConfigurator::getInstance();
    return configurator.setProcessState(new_state);
  });

#ifdef SCALOPUS_TRACING_HAVE_LTTNG
  py::module lttng = tracing.def_submodule("lttng", "The lttng specific components.");
  lttng.def("scope_entry", &lttng::scope_entry);
  lttng.def("scope_exit", &lttng::scope_exit);
  lttng.def("mark_event", &lttng::mark_event);

  py::class_<LttngProvider, LttngProvider::Ptr, TraceEventProvider> lttng_provider(lttng, "LttngProvider");
  lttng_provider.def(py::init<std::string, EndpointManager::Ptr>());
#endif

  native.def("scope_entry", &native::scope_entry);
  native.def("scope_exit", &native::scope_exit);
  native.def("mark_event", &native::mark_event);

  // Add the nop tracepoints for completeness.
  py::module nop = tracing.def_submodule("nop", "The nop specific components.");
  nop.def("scope_entry", &nop::scope_entry);
  nop.def("scope_exit", &nop::scope_exit);
  nop.def("mark_event", &nop::mark_event);

  // Add the providers.

  py::class_<NativeTraceProvider, NativeTraceProvider::Ptr, TraceEventProvider> native_trace_provider(
      native, "NativeTraceProvider");
  native_trace_provider.def(py::init<EndpointManager::Ptr>());
  native_trace_provider.def("receiveEndpoint", &NativeTraceProvider::receiveEndpoint);
  native_trace_provider.def("factory", &NativeTraceProvider::factory);
}
}  // namespace scalopus
