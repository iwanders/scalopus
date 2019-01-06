/*
  Copyright (c) 2019, Ivor Wanders
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

#include "scalopus_catapult/trace_session.h"
#include <iostream>
#include <sstream>

namespace scalopus
{
TraceSession::TraceSession(ResponseFunction&& response_function) : response_(response_function)
{
  worker_ = std::thread([&]() { loop(); });
}

TraceSession::~TraceSession()
{
  running_ = false;
  worker_.join();
}

void TraceSession::addSource(TraceEventSource::Ptr&& source)
{
  sources_.push_back(source);
}

void TraceSession::incoming(const std::string& data)
{
  std::lock_guard<decltype(incoming_mutex_)> lock(incoming_mutex_);
  incoming_msg_.push_back(data);
}

bool TraceSession::haveIncoming() const
{
  std::lock_guard<decltype(incoming_mutex_)> lock(incoming_mutex_);
  return !incoming_msg_.empty();
}

std::string TraceSession::popIncoming()
{
  std::lock_guard<decltype(incoming_mutex_)> lock(incoming_mutex_);
  auto res = incoming_msg_.front();
  incoming_msg_.pop_front();
  return res;
}

void TraceSession::loop()
{
  while (running_)
  {
    // Handle any incoming messages from the websocket
    while (haveIncoming())
    {
      processMessage(popIncoming());
    }

    // Perform work with the sources
    for (auto& source : sources_)
    {
      source->work();
    }

    // Finally, block a bit just to prevent spinning.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  };
}

void TraceSession::processMessage(const std::string& incoming_msg)
{
  std::cout << "[session " << this << "] <- " << incoming_msg << std::endl;
  auto msg = json::parse(incoming_msg);

  if (msg["method"] == "Tracing.getCategories")
  {
    // This method is on the first record click, when we can specify the capture profile.
    // We can specify categories here, this requires folding out in the GUI so it's of limited use unfortunately.
    json res = { { "id", msg["id"] },
                 { "result", { { "categories", { "!foo", "!bar", "disabled-by-default-buz" } } } } };
    outgoing(res.dump());
    return;
  }

  if (msg["method"] == "Tracing.start")
  {
    // Tracing was started by catapult.
    json res = { { "id", msg["id"] }, { "result", {} } };
    for (auto& source : sources_)
    {
      source->startInterval();
    }
    outgoing(res.dump());
    return;
  }

  if (msg["method"] == "Tracing.end")
  {
    // Tracing was stopped.
    for (auto& source : sources_)
    {
      source->stopInterval();
    }

    json res = { { "id", msg["id"] }, { "result", nullptr } };  // nullptr gets converted to NULL, which we need.
    outgoing(res.dump());

    std::vector<json> events;
    for (auto& source : sources_)
    {
      auto results = source->finishInterval();
      events.insert(events.end(), results.begin(), results.end());
    }
    chunkedTransmit(events);

    res = { { "method", "Tracing.tracingComplete" }, { "params", std::vector<int>{} } };
    outgoing(res.dump());
    return;
  }
}

void TraceSession::chunkedTransmit(const std::vector<json>& events)
{
  std::cout << "[session " << this << "] -> events: " << events.size() << std::endl;
  // So, now we send the client data in chunks, needs to be in chunks because the webserver buffer is 16 mb.
  size_t event_count = events.size();
  const size_t chunks_needed = ((event_count / CHUNK_SIZE) + 1);
  for (size_t i = 0; i < chunks_needed; i++)
  {
    size_t start_position = i * CHUNK_SIZE;
    std::vector<json> sliced{
      std::next(events.begin(), start_position),
      std::next(events.begin(), start_position + std::min<size_t>(CHUNK_SIZE, events.size() - start_position))
    };
    response_(formatEvents(sliced));
  }
}

void TraceSession::outgoing(const std::string& msg)
{
  std::cout << "[session " << this << "] -> " << msg << std::endl;
  response_(msg);
}

std::string TraceSession::formatEvents(const std::vector<json>& entries)
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

}  // namespace scalopus
