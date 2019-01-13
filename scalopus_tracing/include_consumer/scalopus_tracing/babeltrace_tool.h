/*
  Copyright (c) 2018, Ivor Wanders
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

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

#ifndef SCALOPUS_TRACING_BABELTRACE_TOOL_H
#define SCALOPUS_TRACING_BABELTRACE_TOOL_H

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "scalopus_tracing/babeltrace_parser.h"
#include "scalopus_tracing/ctfevent.h"
#include "scalopus_tracing/subprocess.hpp"

namespace scalopus
{
/**
 * @brief Class that runs the babeltrace tool and runs the parser on its text output to obtain CTFEvents
 */
class BabeltraceTool
{
public:
  using Ptr = std::shared_ptr<BabeltraceTool>;

  BabeltraceTool();
  ~BabeltraceTool();

  /**
   * @brief Initialise the object, start babeltrace etc.
   * @param path If empty string, connects to the live session as `lttng view test_session`
   *             If string not empty, connects to the live session as `babeltrace path`
                 example path: "net://localhost/host/eagle/scalopus_target_session"
   */
  void init(std::string path = "");

  /**
   * @brief Shuts down the babeltrace tool and parser gracefully.
   */
  void halt();

  /**
   * @brief Create a session and set its callbacks up correctly such that starting record and stopping it adds and
   *        removes its state from the parser.
   */
  std::shared_ptr<BabeltraceParser::EventCallback> addCallback(std::function<void(const CTFEvent& event)> fun);

private:
  std::shared_ptr<subprocess::popen> process_;  //! Babeltrace subprocess.

  BabeltraceParser::Ptr parser_;  //! Parser that consumes stdout from babeltrace.
  std::thread parser_worker_;     //! Worker thread for the parse function.
};
}  // namespace scalopus

#endif
