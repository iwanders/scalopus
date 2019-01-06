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

#ifndef SCALOPUS_CATAPULT_CATAPULT_BACKEND_H
#define SCALOPUS_CATAPULT_CATAPULT_BACKEND_H

#include <seasocks/PageHandler.h>
#include <seasocks/WebSocket.h>
#include <seasocks/util/PathHandler.h>

#include <nlohmann/json.hpp>

#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>

#include "scalopus_catapult/trace_session.h"
#include "scalopus_catapult/endpoint_manager.h"
#include "scalopus_catapult/trace_event_provider.h"

namespace scalopus
{
using json = nlohmann::json;
namespace ss = seasocks;

/**
 * @brief The actual devtools protocol endpoint, this acts as both the webserver and websocket endpoint.
 */
class CatapultBackend : public ss::PageHandler, public ss::WebSocket::Handler
{
public:
  using Ptr = std::shared_ptr<CatapultBackend>;
  using Runnable = std::function<void()>;
  using ExecuteFunction = std::function<void(Runnable&&)>;


  CatapultBackend(const std::vector<TraceEventProvider::Ptr>& providers);

  void setExecutor(ExecuteFunction executor);

  // from PageHandler, this serves the http pages.
  std::shared_ptr<ss::Response> handle(const ss::Request& request) override;

  // from WebSocket::Handler.
  void onConnect(ss::WebSocket* connection) override;
  void onData(ss::WebSocket* connection, const char* data) override;
  void onData(ss::WebSocket* connection, const uint8_t* data, size_t length) override;
  void onDisconnect(ss::WebSocket* connection) override;

private:
  std::mutex session_mutex_;                                //! Mutex for the session map.
  std::map<ss::WebSocket*, TraceSession::Ptr> sessions_;  //! Map of sessions by websockets.
  std::vector<TraceEventProvider::Ptr> providers_;   //!< List of available data providers.
  ExecuteFunction executor_;  //!< Function that allows registering functions for execution on the server thread.

  /**
   * @brief Make a session for this new websocket.
   * @param ws The websocket to allocate a new session for.
   */
  void makeSession(ss::WebSocket* ws);

  /**
   * @brief Retrieve the session associated to this websocket
   * @param ws The websocket to retrieve the session for.
   * @return The devtools session that is associated to this websocket session.
   */
  TraceSession::Ptr getSession(ss::WebSocket* ws);

  /**
   * @brief Removes a session from the session map.
   * @param ws The websocket to remove.
   */
  void delSession(ss::WebSocket* ws);

};

}  // namespace scalopus
#endif  // SCALOPUS_CATAPULT_CATAPULT_BACKEND_H
