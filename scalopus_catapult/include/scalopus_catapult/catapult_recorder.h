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
#ifndef SCALOPUS_CATAPULT_CATAPULT_RECORDER_H
#define SCALOPUS_CATAPULT_CATAPULT_RECORDER_H

#include <scalopus_interface/trace_event_provider.h>
#include <thread>
#include <atomic>
#include <mutex>

namespace scalopus
{

/**
 * @brief The catapult recorder, runs a recording of tracepoints and allows writing to file on destruction.
 */
class CatapultRecorder
{
public:
  using Ptr = std::shared_ptr<CatapultRecorder>;

  /**
   * @brief Construct the catapult recorder.
   */
  CatapultRecorder() = default;

  /**
   * @brief Add a provider, this should be done before the start is invoked and the sources are made.
   */
  void addProvider(TraceEventProvider::Ptr provider);

  /**
   * @brief Start the worker and make the sources from the providers.
   */
  void start();

  /**
   * @brief Stop the worker and clear the sources.
   */
  void stop();

  /**
   * @brief Call startInterval on all the sources.
   */
  void startInterval();

  /**
   * @brief Call stopInterval on all the sources.
   */
  void stopInterval();

  /**
   * @brief If this file is specified the recorder will dump the entire recording to this file when it is destroyed.
   */
  void setupDumpFile(const std::string& file_path);

  /**
   * @brief Stop the interval and write to this file.
   */
  void writeToFile(const std::string& file_path);

  /**
   * @brief stop the interval, finalize it to collect the trace events and return the json serialized results.
   */
  std::string collectEvents();

  // Destructor, used to dump the file.
  ~CatapultRecorder();
private:
  std::vector<TraceEventProvider::Ptr> providers_;
  std::vector<TraceEventSource::Ptr> sources_;

  std::string dump_file_path_;

  std::thread worker_;
  void loop();
  std::atomic_bool running_{ false };
  std::mutex source_mutex_;
  
};

}  // namespace scalopus
#endif  // SCALOPUS_CATAPULT_CATAPULT_RECORDER_H
