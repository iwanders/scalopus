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
#include <nlohmann/json.hpp>
#include <iostream>

namespace scalopus
{
namespace py = pybind11;
using json = nlohmann::json;
/**
 * @brief Convert a json variable into a python object.
 */
py::object jsonToPy(const nlohmann::json& data);

/**
 * @brief Convert a python object into a json variable.
 */
json pyToJson(const py::object& data);
}  // namespace scalopus

namespace pybind11
{
void from_json(const nlohmann::json& data, object& python_value);
void to_json(const object& data, nlohmann::json& json_value);

// Bindings for the pybind11.
namespace detail
{
template <>
struct type_caster<nlohmann::json>
{
public:
  PYBIND11_TYPE_CASTER(nlohmann::json, _("json"));

  /**
   * Conversion part 1 (Python->C++):
   */
  bool load(handle src, bool)
  {
    // Cast it to an object, which works for data containers.
    if (!isinstance<object>(src))
        return false;
    object s = reinterpret_borrow<object>(src);
    // Then, value is a member attribute from the PYBIND11_TYPE_CASTER, we write into that the ooutput of pyToJson
    value = scalopus::pyToJson(s);
    return true;
  }

  /**
   * Conversion part 2 (C++ -> Python)
   */
  static handle cast(nlohmann::json src, return_value_policy /* policy */, handle /* parent */)
  {
    object thing = scalopus::jsonToPy(src);
    return thing.release();
  }
};
}  // namespace detail
}  // namespace pybind11
