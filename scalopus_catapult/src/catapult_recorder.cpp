/*
  Copyright (c) 2018-2019, 2021, Ivor Wanders
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
#include "scalopus_catapult/catapult_recorder.h"
#include <fstream>
#include <sstream>

namespace scalopus
{
void CatapultRecorder::loop()
{
  while(running_)
  {
    {
      std::lock_guard<std::mutex> lock{source_mutex_};
      for (auto& source : sources_)
      {
        source->work();
      }
    }

    // Finally, block a bit just to prevent spinning.
    // @TODO Think of a way to allow it to block here until there are messages or work needs to be done.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void CatapultRecorder::stop()
{
  running_ = false;
  worker_.join();
  sources_.clear();
}


void CatapultRecorder::startInterval()
{
  std::lock_guard<std::mutex> lock{source_mutex_};
  for (auto& source : sources_)
  {
    source->startInterval();
  }
}

void CatapultRecorder::stopInterval()
{
  std::lock_guard<std::mutex> lock{source_mutex_};
  for (auto& source : sources_)
  {
    source->stopInterval();
  }
}

void CatapultRecorder::addProvider(TraceEventProvider::Ptr provider)
{
  providers_.push_back(provider);
}

void CatapultRecorder::start()
{
  if (running_)
  {
    return;
  }
  sources_.clear();
  sources_.reserve(providers_.size());
  // Add all the sources to the session
  for (auto& provider : providers_)
  {
    sources_.push_back(provider->makeSource());
  }
  running_ = true;
  worker_ = std::thread{[&]{loop();}};
}


void CatapultRecorder::setupDumpFile(const std::string& file_path)
{
  dump_file_path_ = file_path;
}


std::string CatapultRecorder::collectEvents()
{
  // Stop the interval first.
  stopInterval();

  // Then collect the events by calling finishInterval
  std::vector<json> events;
  {
    std::lock_guard<std::mutex> lock{source_mutex_};
    for (auto& source : sources_)
    {
      auto results = source->finishInterval();
      events.insert(events.end(), results.begin(), results.end());
    }
  }

  // And format it to json.
  std::stringstream ss;
  ss << "[\n";
  bool has_added = false;
  for (const auto& entry : events)
  {
    if (has_added)
    {
      ss << ",\n";
    }
    has_added = true;
    ss << entry.dump();
  }
  ss << "\n]\n";
  return ss.str();
}

void CatapultRecorder::writeToFile(const std::string& file_path)
{
  const auto output = collectEvents();
  std::ofstream out{file_path};
  out << output;
}

CatapultRecorder::~CatapultRecorder()
{
  stop();
  if (!dump_file_path_.empty())
  {
    writeToFile(dump_file_path_);
  }
}

}  // namespace scalopus
