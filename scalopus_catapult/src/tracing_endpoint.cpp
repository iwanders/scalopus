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

#include "scalopus_catapult/tracing_endpoint.h"

namespace scalopus
{
std::shared_ptr<ss::Response> TracingEndpoint::handle(const ss::Request& request)
{
  if (request.getRequestUri() == "/json/version")
  {
    // This URL is retrieved on the chrome://inspect?tracing page and it is the bold text.
    Json j2 = {
      { "Browser", "Scalopus Devtools Target" },
      { "Protocol-Version", "1.2" },
      { "User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu "
                      "Chromium/64.0.3282.119 Chrome/64.0.3282.119 Safari/537.36" },
      { "V8-Version", "6.4.388.40" },
      { "WebKit-Version", "537.36 (@2ba93af511033c75c55cac08672b808e2c3fbe71)" },
      { "webSocketDebuggerUrl", "ws://127.0.0.1:9090/devtools/page/bar" },
    };

    return ss::Response::htmlResponse(j2.dump());
  }

  if (request.getRequestUri() == "/json")
  {
    // This fakes a page display on this target, this makes it clear the target active and available.
    Json j2 = { {
        // chrome://inspect/inspect.js
        { "description", "Scalopus Operational" },
        { "devtoolsFrontendUrl", "/devtools/inspector.html?ws=127.0.0.1:9222/devtools/p" },
        { "id", "fooo" },
        { "title", "click 'trace' -----------------------------------^" },
        { "type", "Scalopus remote tracing target" },
        { "url", "z" },
        //  { "faviconUrl", DEVTOOLS_ENDPOINT_FAVICON },
        { "webSocketDebuggerUrl", "-" },
    } };
    return ss::Response::htmlResponse(j2.dump());
  }

  return ss::Response::unhandled();
}

void TracingEndpoint::onConnect(ss::WebSocket* ws)
{
  std::cout << "Connection opened for " << ws << std::endl;
  makeSession(ws);
}

void TracingEndpoint::onData(ss::WebSocket* /* connection */, const uint8_t* /* data */, size_t /* length */)
{
  std::cout << "Binary data is not handled." << std::endl;
}

void TracingEndpoint::onData(ss::WebSocket* ws, const char* data)
{
  auto session = getSession(ws);

  if (session == nullptr)
  {
    std::cout << "No session could be found for this websocket." << std::endl;
    return;  // no session could be found, issue.
  }

  std::cout << "Incoming data: " << data << std::endl;

  auto msg = Json::parse(data);

  if (msg["method"] == "Tracing.getCategories")
  {
    // This method is on the first record click, when we can specify the capture profile.
    // We can specify categories here, but they client does also show the default catapult categories.
    // So it's of limited use.
    Json res = { { "id", msg["id"] },
                 { "result", { { "categories", { "!foo", "!bar", "disabled-by-default-lololo" } } } } };
    ws->send(res.dump());
  }

  if (msg["method"] == "Tracing.start")
  {
    // Tracing was started by catapult.
    Json res = { { "id", msg["id"] }, { "result", {} } };
    session->start();
    ws->send(res.dump());
    std::cout << res.dump() << std::endl;
    std::cout << "Tracing started on " << ws << std::endl;
  }

  // self.send(json.dumps({"method":"Tracing.bufferUsage","params":{"percentFull":10,"eventCount":512,"value":10}}))
  // send one percent notice to show that we actually have started doing something.
  ws->send(makeBufferUsage(0.01));

  if (msg["method"] == "Tracing.end")
  {
    // Tracing was stopped.
    session->stop();
    std::cout << "Tracing stopped on " << ws << std::endl;

    Json res = { { "id", msg["id"] }, { "result", nullptr } };
    ws->send(res.dump());

    // conjure data and pass this as a tracingComplete field.
    // So, now we send the client data in chunks, needs to be in chunks because the webserver buffer is 16 mb.
    Json data_collected;
    auto events = session->events();
    size_t event_count = events.size();
    const size_t chunk_size = 10000;
    const size_t chunks_needed = ((event_count / chunk_size) + 1);
    for (size_t i = 0; i < chunks_needed; i++)
    {
      size_t start_position = i * chunk_size;
      std::vector<Json> sliced{
        std::next(events.begin(), start_position),
        std::next(events.begin(), start_position + std::min<size_t>(chunk_size, events.size() - start_position))
      };
      ws->send(formatCollectedData(sliced));
    }

    // send the metadata.
    ws->send(formatCollectedData(session->metadata()));

    std::cout << "Tracing data on " << ws << " contained " << event_count << " events. " << std::endl;

    res = { { "method", "Tracing.tracingComplete" }, { "params", std::vector<int>{} } };
    std::cout << res.dump() << std::endl;
    ws->send(res.dump());
  }
}

std::string TracingEndpoint::formatCollectedData(std::vector<Json> entries)
{
  std::stringstream ss;
  ss << "{ \"method\": \"Tracing.dataCollected\", \"params\": { \"value\": [\n";
  bool has_added = false;
  for (const auto& entry : entries)
  {
    if (has_added)
    {
      ss << ",\n";
    }
    has_added = true;
    ss << entry.dump();
  }
  ss << "]}}";
  return ss.str();
}

std::string TracingEndpoint::makeBufferUsage(double value)
{
  Json tmp;
  tmp["method"] = "Tracing.bufferUsage";
  tmp["params"] = { { "percentFull", 0.05 }, { "eventCount", 512 }, { "value", value } };  // value is actually
                                                                                           // displayed %.
  return tmp.dump();
}

void TracingEndpoint::onDisconnect(ss::WebSocket* ws)
{
  // Websocket was disconnected, remove the session.
  delSession(ws);
}

void TracingEndpoint::init(std::string path)
{

  // Start the tracing tool.
  tracing_tool_ = std::make_shared<BabeltraceTool>();
  tracing_tool_->init(path);
}

TracingEndpoint::TracingEndpoint()
{
}

TracingSession::Ptr TracingEndpoint::getSession(ss::WebSocket* ws)
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

void TracingEndpoint::delSession(ss::WebSocket* ws)
{
  std::lock_guard<std::mutex> lock(session_mutex_);
  auto it = sessions_.find(ws);
  if (it != sessions_.end())
  {
    sessions_.erase(it);
  }
}

void TracingEndpoint::makeSession(ss::WebSocket* ws)
{
  std::lock_guard<std::mutex> lock(session_mutex_);
  auto it = sessions_.find(ws);
  if (it == sessions_.end())
  {
    // New websocket, update the mappings.
    //  mapping_client_->updateServiceList();
    //  mapping_client_->setupMappingRetrieval();
    //  auto mappinglist = mapping_client_->getMappings();
    //  auto mappings = TracingSession::convertMappings(mappinglist);
    sessions_[ws] = std::make_shared<TracingSession>(tracing_tool_);
  }
}

}  // namespace scalopus
