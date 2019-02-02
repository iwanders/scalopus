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
#include "scalopus_general.h"
#include <pybind11/stl.h>
#include <scalopus_general/endpoint_manager_poll.h>
#include <scalopus_general/general.h>
#include <scalopus_interface/endpoint_manager.h>

namespace scalopus
{
namespace py = pybind11;
void add_scalopus_general(py::module& m)
{
  py::module general = m.def_submodule("general", "The general components.");
  py::class_<EndpointIntrospect, EndpointIntrospect::Ptr, Endpoint> endpoint_introspect(general, "EndpointIntrospect");
  endpoint_introspect.def(py::init<>());
  endpoint_introspect.def("supported", &EndpointIntrospect::supported);
  endpoint_introspect.def_property_readonly_static("name", [](py::object /* self */) { return EndpointIntrospect::name; });
  endpoint_introspect.def("factory", &EndpointIntrospect::factory);

  py::class_<EndpointProcessInfo::ProcessInfo> endpoint_process_info_info(general, "ProcessInfo");
  endpoint_process_info_info.def_readwrite("name", &EndpointProcessInfo::ProcessInfo::name);
  endpoint_process_info_info.def_readwrite("threads", &EndpointProcessInfo::ProcessInfo::threads);

  py::class_<EndpointProcessInfo, EndpointProcessInfo::Ptr, Endpoint> endpoint_process_info(general,
                                                                                            "EndpointProcessInfo");
  endpoint_process_info.def(py::init<>());
  endpoint_process_info.def("setProcessName", &EndpointProcessInfo::setProcessName);
  endpoint_process_info.def("processInfo", &EndpointProcessInfo::processInfo);
  endpoint_process_info.def_property_readonly_static("name", [](py::object /* self */) { return EndpointProcessInfo::name; });
  endpoint_process_info.def("factory", &EndpointProcessInfo::factory);

  general.def("setThreadName", [](const std::string& name) { ThreadNameTracker::getInstance().setCurrentName(name); });

  py::class_<EndpointManagerPoll, EndpointManagerPoll::Ptr, EndpointManager> endpoint_manager_poll(
      general, "EndpointManagerPoll");
  endpoint_manager_poll.def(py::init<TransportFactory::Ptr>());
  endpoint_manager_poll.def("endpoints", &EndpointManagerPoll::endpoints);

  // @TODO(iwanders): Test this method.
  endpoint_manager_poll.def("addEndpointFactory", [](EndpointManagerPoll& manager, std::string name, py::object fun) {
    manager.addEndpointFactory(name, [fun](const Transport::Ptr& transport) {
      py::object result_py = fun(transport);
      Endpoint::Ptr endpoint = result_py.cast<Endpoint::Ptr>();
      if (endpoint == nullptr)
      {
        throw py::value_error("Could not cast returned object to Endpoint.");
      }
      return endpoint;
    });
  });
  endpoint_manager_poll.def("startPolling", &EndpointManagerPoll::startPolling);
  endpoint_manager_poll.def("stopPolling", &EndpointManagerPoll::stopPolling);
  endpoint_manager_poll.def("manage", &EndpointManagerPoll::manage);
}
}  // namespace scalopus
