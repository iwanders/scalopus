#pragma once
#include <pybind11/pybind11.h>

// Python part of subclassed objects get destroyed when an subclass is created from Python and handed back to C++.

// Multiple issues / pull requests govern it.
// https://github.com/pybind/pybind11/issues/1389
// https://github.com/pybind/pybind11/pull/1566/
// https://github.com/pybind/pybind11/issues/1546
// https://github.com/pybind/pybind11/issues/1333

namespace scalopus
{
class TraceEventSource;
}
// https://github.com/pybind/pybind11/issues/1546
namespace pybind11
{
namespace py = pybind11;
namespace detail
{

  template<>
  struct type_caster<std::shared_ptr<scalopus::TraceEventSource>>
  {
    PYBIND11_TYPE_CASTER (std::shared_ptr<scalopus::TraceEventSource>, _("scalopus::TraceEventSource"));

    using TraceEventSourceCaster = copyable_holder_caster<scalopus::TraceEventSource, std::shared_ptr<scalopus::TraceEventSource>>;

    bool load (pybind11::handle src, bool b)
    {
      TraceEventSourceCaster bc;
      bool success = bc.load (src, b);
      if (!success)
      {
        return false;
      }

      auto py_obj = py::reinterpret_borrow<py::object> (src);
      auto base_ptr = static_cast<std::shared_ptr<scalopus::TraceEventSource>> (bc);
      auto py_obj_ptr = std::make_shared<py::object> (py_obj);

      value = std::shared_ptr<scalopus::TraceEventSource> (py_obj_ptr, base_ptr.get ());
      return true;
    }

    static handle cast (std::shared_ptr<scalopus::TraceEventSource> base,
                        return_value_policy rvp,
                        handle h)
    {
      return TraceEventSourceCaster::cast (base, rvp, h);
    }
  };

  template <>
  struct is_holder_type<scalopus::TraceEventSource, std::shared_ptr<scalopus::TraceEventSource>> : std::true_type {};
}
}