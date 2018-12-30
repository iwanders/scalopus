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

#ifndef SCALOPUS_CATAPULT_TRACING_SESSION_H
#define SCALOPUS_CATAPULT_TRACING_SESSION_H

#include <scalopus_lttng/babeltrace_tool.h>
#include "scalopus_catapult/endpoint_manager.h"
#include <nlohmann/json.hpp>
#include <map>

namespace scalopus
{
using json = nlohmann::json;

/**
 * @brief The session that is used to hangle / process one websocket.
 */
class TracingSession
{
public:
  using Ptr = std::shared_ptr<TracingSession>;
  /**
   * @brief On websocket connect the session is initiated.
   * @param mappings The mappings as retrieved from the services calls and converted with convertMappings.
   * @param babel_session The session to obtain the events from.
   */
  TracingSession(BabeltraceTool::Ptr tool, EndpointManager::Ptr manager);

  /**
   * @brief When recording is started in the catapult interface, this is called.
   */
  void start();

  /**
   * @brief When recording is stopped in the catapult interface, this is called.
   */
  void stop();

  /**
   * @brief Convert the recorded events present in the babel_session into the trace event format accepted by catapult.
   * @return Vector of json representations for each event.
   * @warn The json representation MUST have newline after every trace event, otherwise catapult doesn't accept it.
   */
  std::vector<json> events();

  /**
   * @brief Return a vector of json objects representing the metadata.
   */
  std::vector<json> metadata();

private:
  BabeltraceTool::Ptr tool_;
  EndpointManager::Ptr manager_;
  std::vector<CTFEvent> events_;
  std::shared_ptr<BabeltraceParser::EventCallback> callback_;

  void updateMapping();
  std::map<unsigned int, std::string> trace_mapping_;
};

}  // namespace scalopus
#endif
