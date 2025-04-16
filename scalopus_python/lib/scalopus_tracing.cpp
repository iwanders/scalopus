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

  // Start EndpointTraceMapping
  py::class_<EndpointTraceMapping, EndpointTraceMapping::Ptr, Endpoint> endpoint_trace_mapping(tracing,
                                                                                               "EndpointTraceMapping");
  endpoint_trace_mapping.def(py::init<>());
  endpoint_trace_mapping.def("mapping", &EndpointTraceMapping::mapping);
  endpoint_trace_mapping.def_static("factory", &EndpointTraceMapping::factory);
  endpoint_trace_mapping.def_property_readonly_static("name",
                                                      [](py::object /* self */) { return EndpointTraceMapping::name; });
  // End EndpointTraceMapping

  // Start EndpointTraceConfigurator

  py::class_<EndpointTraceConfigurator, EndpointTraceConfigurator::Ptr, Endpoint> endpoint_tc(
      tracing, "EndpointTraceConfigurator");
  endpoint_tc.def(py::init<>());
  endpoint_tc.def("setTraceState", &EndpointTraceConfigurator::setTraceState);
  endpoint_tc.def("getTraceState", &EndpointTraceConfigurator::getTraceState);
  endpoint_tc.def_property_readonly_static("name",
                                           [](py::object /* self */) { return EndpointTraceConfigurator::name; });
  endpoint_tc.def_static("factory", &EndpointTraceConfigurator::factory);

  py::class_<EndpointTraceConfigurator::TraceConfiguration> endpoint_tc_trace_conf(endpoint_tc, "TraceConfiguration");
  endpoint_tc_trace_conf.def(py::init<>());
  endpoint_tc_trace_conf.def_readwrite("process_state", &EndpointTraceConfigurator::TraceConfiguration::process_state);
  endpoint_tc_trace_conf.def_readwrite("cmd_success", &EndpointTraceConfigurator::TraceConfiguration::cmd_success);
  endpoint_tc_trace_conf.def_readwrite("set_process_state",
                                       &EndpointTraceConfigurator::TraceConfiguration::set_process_state);
  endpoint_tc_trace_conf.def_readwrite("new_thread_state",
                                       &EndpointTraceConfigurator::TraceConfiguration::new_thread_state);
  endpoint_tc_trace_conf.def_readwrite("set_new_thread_state",
                                       &EndpointTraceConfigurator::TraceConfiguration::set_new_thread_state);
  endpoint_tc_trace_conf.def_readwrite("thread_state", &EndpointTraceConfigurator::TraceConfiguration::thread_state);
  // For some reason, assigning into tread_state directly didn't work, make a simple assign function.
  endpoint_tc_trace_conf.def("add_thread_entry", [](EndpointTraceConfigurator::TraceConfiguration& v, unsigned long id,
                                                    bool state) { v.thread_state[id] = state; });

  endpoint_tc_trace_conf.def("to_dict", [](const EndpointTraceConfigurator::TraceConfiguration& p) {
    auto dict = py::dict();
    dict["set_process_state"] = p.set_process_state;
    dict["process_state"] = p.process_state;
    dict["set_new_thread_state"] = p.set_new_thread_state;
    dict["new_thread_state"] = p.new_thread_state;
    dict["cmd_success"] = p.cmd_success;
    dict["thread_state"] = p.thread_state;
    return dict;
  });
  // End EndpointTraceConfigurator

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
    auto configurator = TraceConfigurator::getInstance();
    return configurator->getThreadState();
  });
  tracing.def("setThreadState", [](bool new_state) {
    auto configurator = TraceConfigurator::getInstance();
    return configurator->setThreadState(new_state);
  });
  tracing.def("getProcessState", []() {
    auto configurator = TraceConfigurator::getInstance();
    return configurator->getProcessState();
  });
  tracing.def("setProcessState", [](bool new_state) {
    auto configurator = TraceConfigurator::getInstance();
    return configurator->setProcessState(new_state);
  });

#ifdef SCALOPUS_TRACING_HAVE_LTTNG
  py::module lttng = tracing.def_submodule("lttng", "The lttng specific components.");
  lttng.def("scope_entry", &lttng::scope_entry);
  lttng.def("scope_exit", &lttng::scope_exit);
  lttng.def("mark_event", &lttng::mark_event);
  lttng.def("count_event", &lttng::count_event);

  py::class_<LttngProvider, LttngProvider::Ptr, TraceEventProvider> lttng_provider(lttng, "LttngProvider");
  lttng_provider.def(py::init<std::string, EndpointManager::Ptr>());
#endif

  native.def("scope_entry", &native::scope_entry);
  native.def("scope_exit", &native::scope_exit);
  native.def("mark_event", &native::mark_event);
  native.def("count_event", &native::count_event);

  // Add the nop tracepoints for completeness.
  py::module nop = tracing.def_submodule("nop", "The nop specific components.");
  nop.def("scope_entry", &nop::scope_entry);
  nop.def("scope_exit", &nop::scope_exit);
  nop.def("mark_event", &nop::mark_event);
  nop.def("count_event", &nop::count_event);

  // Add the providers.

  py::class_<NativeTraceProvider, NativeTraceProvider::Ptr, TraceEventProvider> native_trace_provider(
      native, "NativeTraceProvider");
  native_trace_provider.def(py::init<EndpointManager::Ptr>());
  native_trace_provider.def("receiveEndpoint", &NativeTraceProvider::receiveEndpoint);
  native_trace_provider.def("factory", &NativeTraceProvider::factory);
}
}  // namespace scalopus
