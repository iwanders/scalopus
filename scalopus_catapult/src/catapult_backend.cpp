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
#include "catapult_backend.h"

// This is a 32x32, 16 color .ico of the :tada: emoji.
#define DEVTOOLS_ENDPOINT_FAVICON                                                                                      \
  "data:image/x-icon;base64,"                                                                                          \
  "AAABAAEAICAQAAEABADoAgAAFgAAACgAAAAgAAAAQAAAAAEABAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADxL5sA5GG+AIJh/gAadfUA/o9hAK"     \
  "6EpAC5g+kANZ/eABu7+wCExO8A/860ANS+/gC80+sATuT9APv8/AAAAAAA7tm+7u7u7u7u7u7u7u7u7sghG87u7u7u7u7u7u7u7u52AZ3Ynu7u"     \
  "7u7u7u7u7r7uwBiN3dic7u7u7u7u7utr7lF4iN2REa7u7u7u7u7ua+7liIhxEbl3nu7u7u7u7u7u54eJABnd3Xeezuqu7uye7unYMAHZ3d3Yd8"     \
  "ykTu7nju7sdwAYiN3d2WAEyu7u6c7u7nEFeIiN3bths3nO7u7uzuzACDiIiJERud2Hd87u6Xnu4Jh4iIYBHN3d2Id5nO6e7uXYiIhgAZ3d3d04"     \
  "iN2e7u7piIh2ABfY3d2Yg4md7u7rJXh4YAbYjX3YiIiO7u7u7iJ4dQBoiI3YwMeI7u7u7u6+x3AGiIiIiEQEXK7u7u7u7udQGIiIjYjYpERKqq"     \
  "7u7u7lEXiIiIjd3dyqRERERO7uRRl4iIM4jdie7u6qqkru7qBdiIgzKN2M5r7u7u7u7u7kWIiIgyiIzuJu7u7u7u7u6ljYhzMojO7mvu7O7u7k"     \
  "RESniIeDJ+ru4r7Css7upO7u6Yg3My5ErsJrbiLu7upK7s6DN3IupO7mK+wi7u7uRE7dl5nibu7u7u7rLra+pEBO2esu4i7u7u7u7iZiJkSq7u"     \
  "7rYrJq7u7u7rvu7iJE7u7u7rsiRE7u7u4ivu5ipK7u7u4iK6ru7u7uu+7ra+6u7u7usr7u7u7u7u7u7r4AAAAAAAAAAAAAAAAAAAAAAAAAAAAA"     \
  "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"     \
  "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=="

namespace scalopus
{
void CatapultBackend::addProvider(TraceEventProvider::Ptr provider)
{
  providers_.push_back(std::move(provider));
}

std::shared_ptr<ss::Response> CatapultBackend::handle(const ss::Request& request)
{
  if (request.getRequestUri() == "/json/version")
  {
    // This URL is retrieved on the chrome://inspect?tracing page and it is the bold text.
    json j2 = {
      { "Browser", "Scalopus Devtools Target" },
      { "Protocol-Version", "1.2" },
      { "User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu "
                      "Chromium/64.0.3282.119 Chrome/64.0.3282.119 Safari/537.36" },
      { "V8-Version", "6.4.388.40" },
      { "WebKit-Version", "537.36 (@2ba93af511033c75c55cac08672b808e2c3fbe71)" },
      { "webSocketDebuggerUrl", "devtools/page/bar" },
    };

    return ss::Response::htmlResponse(j2.dump());
  }

  if (request.getRequestUri() == "/json")
  {
    // This fakes a page display on this target, this makes it clear the target active and available.
    json j2 = { {
        // chrome://inspect/inspect.js
        { "description", "Scalopus Operational" },
        { "devtoolsFrontendUrl", "/devtools/inspector.html?ws=127.0.0.1:9222/devtools/p" },
        { "id", "fooo" },
        { "title", "click 'trace' -----------------------------------------^" },
        { "type", "Scalopus remote tracing target" },
        { "url", "z" },
        { "faviconUrl", DEVTOOLS_ENDPOINT_FAVICON },
        { "webSocketDebuggerUrl", "-" },
    } };
    return ss::Response::htmlResponse(j2.dump());
  }

  return ss::Response::unhandled();
}

void CatapultBackend::onConnect(ss::WebSocket* ws)
{
  logger_("[CatapultBackend]: New connection opened: " + std::to_string(reinterpret_cast<std::size_t>(ws)));
  makeSession(ws);
}

void CatapultBackend::onData(ss::WebSocket* /* connection */, const uint8_t* /* data */, size_t /* length */)
{
  logger_("[CatapultBackend]: Binary data is not handled.");
}

void CatapultBackend::onData(ss::WebSocket* ws, const char* data)
{
  auto session = getSession(ws);

  if (session == nullptr)
  {
    logger_("[CatapultBackend]: No session could be found for this websocket." +
            std::to_string(reinterpret_cast<std::size_t>(ws)));
    return;  // no session could be found, issue.
  }
  session->incoming(data);
}

void CatapultBackend::onDisconnect(ss::WebSocket* ws)
{
  // Websocket was disconnected, remove the session.
  delSession(ws);
}

TraceSession::Ptr CatapultBackend::getSession(ss::WebSocket* ws)
{
  // Retrieve the session for this websocket.
  std::lock_guard<std::mutex> lock(session_mutex_);
  auto it = sessions_.find(ws);
  if (it != sessions_.end())
  {
    return it->second;
  }
  return nullptr;
}

void CatapultBackend::delSession(ss::WebSocket* ws)
{
  std::lock_guard<std::mutex> lock(session_mutex_);
  auto it = sessions_.find(ws);
  if (it != sessions_.end())
  {
    sessions_.erase(it);
  }
}

void CatapultBackend::makeSession(ss::WebSocket* ws)
{
  std::lock_guard<std::mutex> lock(session_mutex_);
  auto it = sessions_.find(ws);
  if (it == sessions_.end())
  {
    // New sessions, create the tracing session to use.
    // This function must only sent if the websocket is actually still availble to send to...
    // To guarantee this we create a weak pointer to the trace session, which we capture in the lambda.
    auto response_function = [ws, this](const std::string& data) {
      auto runnable = [ws, data, this]() {
        // lock the session mutex, technically this is not necessary as this function is executed from the server thread
        // and connections will not be closed or opened while this function is executing.
        std::lock_guard<std::mutex> session_lock(session_mutex_);
        auto session_it = sessions_.find(ws);
        if (session_it != sessions_.end())
        {
          session_it->first->send(data);  // success
        }
      };
      executor_(runnable);
    };
    // Create the session.
    auto session = std::make_shared<TraceSession>(std::move(response_function));
    session->setLogger(logger_);

    // Add all the sources to the session
    for (auto& provider : providers_)
    {
      session->addSource(provider->makeSource());
    }
    sessions_[ws] = std::move(session);
  }
}

void CatapultBackend::setExecutor(ExecuteFunction executor)
{
  executor_ = executor;
}

void CatapultBackend::setLogger(LoggingFunction logger)
{
  logger_ = std::move(logger);
}

}  // namespace scalopus
