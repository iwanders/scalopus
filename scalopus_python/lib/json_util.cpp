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
#include "json_util.h"
#include <iostream>

namespace pybind11_nlohmann_conversion
{
using json = nlohmann::json;
namespace py = pybind11;

py::object jsonToPy(const json& data)
{
  // Superset checks
  // is_primitive:  (string, number, boolean, or null).
  // is_structured: (array or object)
  // is_number: This includes both integer (signed and unsigned) and floating-point values.
  // is_object: map-esque.
  // is_array: vector/list-esque.
  // is_number_integer: Unsigned or signed integer.
  // is_discarded: Placed if the parser discarded an entry.

  // Which leaves types:
  // is_null: None
  // is_boolean: bool
  // is_number_unsigned: uint
  // is_number_integer: int
  // is_number_float: double
  // is_string: string

  //  std::cout << "jsonToPy: " << data.dump() << std::endl;

  if (data.is_null())
  {
    return py::none();
  }

  if (data.is_boolean())
  {
    return py::bool_(data.get<bool>());
  }

  if (data.is_number_unsigned())  // check unsigned first, this is subset of integer.
  {
    return py::int_(data.get<std::uint64_t>());
  }

  if (data.is_number_integer())
  {
    return py::int_(data.get<std::int64_t>());
  }

  if (data.is_number_float())
  {
    return py::float_(data.get<double>());
  }

  if (data.is_string())
  {
    return py::str(data.get<std::string>());
  }

  // Handle lists.
  if (data.is_array())
  {
    py::list res;
    for (const auto& v : data)
    {
      res.append(jsonToPy(v));
    }
    return res;
  }

  // Handle dictionaries.
  if (data.is_object())
  {
    py::dict res;
    for (auto it : data.items())
    {
      res[jsonToPy(it.key())] = jsonToPy(it.value());
    }
    return res;
  }

  if (data.is_discarded())
  {
    return py::object();  // return an empty object if the parser discarded something
  }
  // This can only happen if the json library adds a type.
  throw py::value_error("Cannot convert provided json element to python.");
  return py::none();
}

json pyToJson(const py::object& data)
{
  // Handle types in order from the pybind11 reference. State which implementations are missing.

  // missing iterable

  //  std::cout << "pyToJson: " << std::endl;
  //  py::print(data);

  if (py::isinstance<py::str>(data))
  {
    return data.cast<std::string>();
  }

  if (py::isinstance<py::bytes>(data))
  {
    return data.cast<std::vector<std::uint8_t>>();
  }

  if (py::isinstance<py::none>(data))
  {
    return json{};
  }

  if (py::isinstance<py::bool_>(data))
  {
    return data.cast<bool>();
  }

  if (py::isinstance<py::int_>(data))
  {
    return data.cast<std::int64_t>();
  }

  if (py::isinstance<py::float_>(data))
  {
    return data.cast<double>();
  }
  // missing weakref
  // missing slice
  // missing capsule

  // tuple is handled like a sequence.
  if (py::isinstance<py::sequence>(data))
  {
    json json_seq;
    for (const auto& v : data.cast<py::sequence>())
    {
      json_seq.push_back(pyToJson(v.cast<py::object>()));
    }
    return json_seq;
  }

  if (py::isinstance<py::dict>(data))
  {
    py::dict our_dict = data.cast<py::dict>();
    json our_json_map;
    for (const auto& k_v : our_dict)
    {
      if (!py::isinstance<py::str>(k_v.first))
      {
        // Keys in json MUST be string :(
        throw py::type_error("Keys of dictionaries must be for json conversion.");
      }
      our_json_map[k_v.first.cast<py::str>()] = pyToJson(k_v.second.cast<py::object>());
    }
    return our_json_map;
  }

  // missing list: sequence should handle it.
  // missing args
  // missing kwargs

  if (py::isinstance<py::set>(data))
  {
    std::set<json> json_set;
    for (const auto& v : data.cast<py::set>())
    {
      json_set.insert(pyToJson(v.cast<py::object>()));
    }
    return json_set;
  }
  // missing function
  // missing buffer
  // missing memoryview

  // For missing entries, return json.
  return json{};
}

void add_pybind11_nlohmann_tests(py::module& m)
{
  py::module test_module = m.def_submodule("pybind11_nlohmann_test", "The tests for pybind11 nlohmann conversion.");
  test_module.def("echo", [](const py::object& object) {
    json json_object = object;
    return json_object.get<py::object>();
  });
  test_module.def("json_str", [](const py::object& object) {
    json json_object = object;
    return json_object.dump();
  });
}

}  // namespace pybind11_nlohmann_conversion
