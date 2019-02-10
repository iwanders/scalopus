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
#include <scalopus_interface/endpoint.h>
#include <scalopus_interface/trace_event_provider.h>

// Python part of subclassed objects get destroyed when an subclass is created from Python and handed back to C++.

// Multiple issues / pull requests govern it.
// https://github.com/pybind/pybind11/issues/1389
// https://github.com/pybind/pybind11/pull/1566/
// https://github.com/pybind/pybind11/issues/1546
// https://github.com/pybind/pybind11/issues/1333

// Based on https://github.com/pybind/pybind11/issues/1546
namespace pybind11
{
namespace py = pybind11;
namespace detail
{
// Start of Endpoint
template <>
struct type_caster<std::shared_ptr<scalopus::Endpoint>>
{
  PYBIND11_TYPE_CASTER(std::shared_ptr<scalopus::Endpoint>, _("scalopus::Endpoint"));

  using EndpointCaster = copyable_holder_caster<scalopus::Endpoint, std::shared_ptr<scalopus::Endpoint>>;

  bool load(pybind11::handle src, bool b)
  {
    EndpointCaster bc;
    bool success = bc.load(src, b);
    if (!success)
    {
      return false;
    }

    auto py_obj = py::reinterpret_borrow<py::object>(src);
    auto base_ptr = static_cast<std::shared_ptr<scalopus::Endpoint>>(bc);
    auto py_obj_ptr = std::shared_ptr<py::object>(new py::object{ py_obj }, [](auto py_object_ptr) {
      pybind11::gil_scoped_acquire gil;
      delete py_object_ptr;
    });

    value = std::shared_ptr<scalopus::Endpoint>(py_obj_ptr, base_ptr.get());
    return true;
  }

  static handle cast(std::shared_ptr<scalopus::Endpoint> base, return_value_policy rvp, handle h)
  {
    return EndpointCaster::cast(base, rvp, h);
  }
};

template <>
struct is_holder_type<scalopus::Endpoint, std::shared_ptr<scalopus::Endpoint>> : std::true_type
{
};
// End of Endpoint
// Start of TraceEventProvider
template <>
struct type_caster<std::shared_ptr<scalopus::TraceEventProvider>>
{
  PYBIND11_TYPE_CASTER(std::shared_ptr<scalopus::TraceEventProvider>, _("scalopus::TraceEventProvider"));

  using TraceEventProviderCaster =
      copyable_holder_caster<scalopus::TraceEventProvider, std::shared_ptr<scalopus::TraceEventProvider>>;

  bool load(pybind11::handle src, bool b)
  {
    TraceEventProviderCaster bc;
    bool success = bc.load(src, b);
    if (!success)
    {
      return false;
    }

    auto py_obj = py::reinterpret_borrow<py::object>(src);
    auto base_ptr = static_cast<std::shared_ptr<scalopus::TraceEventProvider>>(bc);
    auto py_obj_ptr = std::shared_ptr<py::object>(new py::object{ py_obj }, [](auto py_object_ptr) {
      pybind11::gil_scoped_acquire gil;
      delete py_object_ptr;
    });

    value = std::shared_ptr<scalopus::TraceEventProvider>(py_obj_ptr, base_ptr.get());
    return true;
  }

  static handle cast(std::shared_ptr<scalopus::TraceEventProvider> base, return_value_policy rvp, handle h)
  {
    return TraceEventProviderCaster::cast(base, rvp, h);
  }
};

template <>
struct is_holder_type<scalopus::TraceEventProvider, std::shared_ptr<scalopus::TraceEventProvider>> : std::true_type
{
};
// End of TraceEventProvider

// Start of TraceEventSource
template <>
struct type_caster<std::shared_ptr<scalopus::TraceEventSource>>
{
  PYBIND11_TYPE_CASTER(std::shared_ptr<scalopus::TraceEventSource>, _("scalopus::TraceEventSource"));

  using TraceEventSourceCaster =
      copyable_holder_caster<scalopus::TraceEventSource, std::shared_ptr<scalopus::TraceEventSource>>;

  bool load(pybind11::handle src, bool b)
  {
    TraceEventSourceCaster bc;
    bool success = bc.load(src, b);
    if (!success)
    {
      return false;
    }

    auto py_obj = py::reinterpret_borrow<py::object>(src);
    auto base_ptr = static_cast<std::shared_ptr<scalopus::TraceEventSource>>(bc);
    auto py_obj_ptr = std::shared_ptr<py::object>(new py::object{ py_obj }, [](auto py_object_ptr) {
      pybind11::gil_scoped_acquire gil;
      delete py_object_ptr;
    });

    value = std::shared_ptr<scalopus::TraceEventSource>(py_obj_ptr, base_ptr.get());
    return true;
  }

  static handle cast(std::shared_ptr<scalopus::TraceEventSource> base, return_value_policy rvp, handle h)
  {
    return TraceEventSourceCaster::cast(base, rvp, h);
  }
};

template <>
struct is_holder_type<scalopus::TraceEventSource, std::shared_ptr<scalopus::TraceEventSource>> : std::true_type
{
};
// End of TraceEventSource

}  // namespace detail
}  // namespace pybind11