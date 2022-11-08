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
#include <scalopus_tracing/tracing.h>
#include <thread>
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
  std::thread thread_;
  bool thread_running_{ false };

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
  void stage_destroy(int ms)
  {
    thread_ = std::thread([&]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      source.reset();
    });
    thread_running_ = true;
  }
  void join()
  {
    py::gil_scoped_release release;
    if (thread_running_)
    {
      thread_.join();
      thread_running_ = false;
    }
  }
  ~PythonSubclasserSpawner()
  {
    join();
  }
};

namespace py = pybind11;
void add_python_test_helpers(py::module& m)
{
  pybind11_nlohmann_conversion::add_pybind11_nlohmann_tests(m);

  py::module test_helpers = m.def_submodule("test_helpers", "Some test helpers.");

  py::class_<PythonSubclasserSpawner, PythonSubclasserSpawner::Ptr> subclass_spawner(test_helpers,
                                                                                     "PythonSubclasserSpawner");
  subclass_spawner.def(py::init<>());
  subclass_spawner.def("addProvider", &PythonSubclasserSpawner::addProvider);
  subclass_spawner.def("makeSourceFrom", &PythonSubclasserSpawner::makeSourceFrom);
  subclass_spawner.def("getSource", &PythonSubclasserSpawner::getSource);
  subclass_spawner.def("resetSource", &PythonSubclasserSpawner::resetSource);
  subclass_spawner.def("call", &PythonSubclasserSpawner::call);
  subclass_spawner.def("stage_destroy", &PythonSubclasserSpawner::stage_destroy);
  subclass_spawner.def("join", &PythonSubclasserSpawner::join);

  test_helpers.def("clearTraceNames", []() { StaticStringTracker::getInstance().clear(); });
}
}  // namespace scalopus
