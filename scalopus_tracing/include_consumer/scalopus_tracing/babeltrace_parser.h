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
#ifndef SCALOPUS_TRACING_BABELTRACE_PARSER_H
#define SCALOPUS_TRACING_BABELTRACE_PARSER_H

#include <atomic>
#include <functional>
#include <iostream>
#include <istream>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "scalopus_tracing/ctfevent.h"

namespace scalopus
{
/**
 * @brief Class that parses line from a stream and passes events to the recording sessions.
 */
class BabeltraceParser
{
public:
  struct EventCallback
  {
    bool active{ true };
    std::function<void(const CTFEvent& event)> callback;

    void disable()
    {
      active = false;
      callback = nullptr;
    }
  };

  using Ptr = std::shared_ptr<BabeltraceParser>;

  BabeltraceParser();

  /**
   * @brief Blocking call that keeps reading from stdout, if there are recording sessions in the set, lines from stdout
   *        are parsed and passed to the recording sessions.
   */
  void process(FILE* stdout);

  /**
   * @brief Return whether the process function is running.
   */
  bool isProcessing();

  /**
   * @brief Causes the loop in process to be broken, allowing return from that function.
   */
  void halt();

  /**
   * @brief Set a sessions state, if record_state is true it is added to the set of recording sessions, otherwise it is
   *        removed.
   */
  void setCallback(const std::shared_ptr<EventCallback>& callback);

  /**
   * @brief Parse a line into a CTFEvent object.
   */
  static CTFEvent parse(std::string line);

private:
  std::atomic_bool processing_;  //! Bool to check on each while loop in the process function.

  std::mutex mutex_;                                             //! Mutex for the recording sessions set.
  std::set<std::shared_ptr<EventCallback>> sessions_recording_;  //! Set of sessions that are actively recording.
};

}  // namespace scalopus

#endif
