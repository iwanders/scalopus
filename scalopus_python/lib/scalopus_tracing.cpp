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
#include <scalopus_tracing/lttng_tracepoint.h>
#endif

#include <scalopus_tracing/nop_tracepoint.h>

#include <scalopus_tracing/native_trace_provider.h>

namespace scalopus
{
namespace py = pybind11;
void add_scalopus_tracing(py::module& m)
{
  py::module tracing = m.def_submodule("tracing", "The tracing specific components.");
  py::class_<EndpointTraceMapping, EndpointTraceMapping::Ptr, Endpoint> endpoint_trace_mapping(tracing,
                                                                                               "EndpointTraceMapping");
  endpoint_trace_mapping.def(py::init<>());
  endpoint_trace_mapping.def("mapping", &EndpointTraceMapping::mapping);
  endpoint_trace_mapping.def("factory", &EndpointTraceMapping::factory);
  endpoint_trace_mapping.def_property_readonly_static("name",
                                                      [](py::object /* self */) { return EndpointTraceMapping::name; });

  py::module native = tracing.def_submodule("native", "The native specific components.");
  py::class_<EndpointNativeTraceSender, EndpointNativeTraceSender::Ptr, Endpoint> endpoint_native_trace_sender(
      native, "EndpointNativeTraceSender");
  endpoint_native_trace_sender.def_property_readonly_static(
      "name", [](py::object /* self */) { return EndpointNativeTraceSender::name; });
  endpoint_native_trace_sender.def(py::init<>());

  tracing.def("setTraceName", [](const unsigned int id, const std::string& name) {
    ScopeTraceTracker::getInstance().insert(id, name);
  });

#ifdef SCALOPUS_TRACING_HAVE_LTTNG
  py::module lttng = tracing.def_submodule("lttng", "The lttng specific components.");
  lttng.def("scope_entry", &lttng::scope_entry);
  lttng.def("scope_exit", &lttng::scope_exit);
#endif

  native.def("scope_entry", &native::scope_entry);
  native.def("scope_exit", &native::scope_exit);

  // Add the nop tracepoints for completeness.
  py::module nop = tracing.def_submodule("nop", "The nop specific components.");
  nop.def("scope_entry", &nop::scope_entry);
  nop.def("scope_exit", &nop::scope_exit);

  // Add the providers.

  py::class_<NativeTraceProvider, NativeTraceProvider::Ptr, TraceEventProvider> native_trace_provider(
      native, "NativeTraceProvider");
  native_trace_provider.def(py::init<EndpointManager::Ptr>());
  native_trace_provider.def("makeSource", &NativeTraceProvider::makeSource);
  native_trace_provider.def("receiveEndpoint", &NativeTraceProvider::receiveEndpoint);
  native_trace_provider.def("factory", &NativeTraceProvider::factory);
}
}  // namespace scalopus
