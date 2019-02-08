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
#include <pybind11/stl.h>
#include <scalopus_catapult/catapult_server.h>
#include "json_util.h"
#include "scalopus_catapult.h"

namespace scalopus
{
class PythonSubclasserSpawner
{
public:
  using Ptr = std::shared_ptr<PythonSubclasserSpawner>;
  TraceEventProvider::Ptr provider;
  TraceEventSource::Ptr source;

  void addProvider(TraceEventProvider::Ptr prov)
  {
    provider = prov;
  }
  void makeSourceFrom()
  {
    source = provider->makeSource();
  }
  TraceEventSource::Ptr getSource()
  {
    return source;
  }
  void call()
  {
    auto res = source->finishInterval();
  }
  void resetSource()
  {
    source.reset();
  }
};

PythonSubclasserSpawner::Ptr stored;

namespace py = pybind11;
void add_python_test_helpers(py::module& m)
{
  pybind11_nlohmann_conversion::add_pybind11_nlohmann_tests(m);

  py::module test_helpers = m.def_submodule("test_helpers", "The catapult components.");

  py::class_<PythonSubclasserSpawner, PythonSubclasserSpawner::Ptr> subclass_spawner(test_helpers,
                                                                                     "PythonSubclasserSpawner");
  subclass_spawner.def(py::init<>());
  subclass_spawner.def("addProvider", &PythonSubclasserSpawner::addProvider);
  subclass_spawner.def("makeSourceFrom", &PythonSubclasserSpawner::makeSourceFrom);
  subclass_spawner.def("getSource", &PythonSubclasserSpawner::getSource);
  subclass_spawner.def("resetSource", &PythonSubclasserSpawner::resetSource);
  subclass_spawner.def("call", &PythonSubclasserSpawner::call);

  test_helpers.def("store", [&](PythonSubclasserSpawner::Ptr thing) { stored = thing; });
  test_helpers.def("clear", [&]() { stored.reset(); });
  test_helpers.def("retrieve", [&]() { return stored; });
}
}  // namespace scalopus
