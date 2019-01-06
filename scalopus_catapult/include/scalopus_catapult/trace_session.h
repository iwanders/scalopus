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

#ifndef SCALOPUS_CATAPULT_TRACE_SESSION_H
#define SCALOPUS_CATAPULT_TRACE_SESSION_H

#include "scalopus_catapult/trace_event_source.h"
#include <functional>
#include <list>
#include <string>
#include <thread>
#include <mutex>

namespace scalopus
{

/**
 * @brief This class is created for every individual websocket connection. It uses its own thread to perform actions and
 *        accepts messages from the websocket. It responds at its discretion or initiates communication from its worker
 *        thread. All data that is sent through the websocket must be sent through the response_function.
 */
class TraceSession
{
  static const size_t CHUNK_SIZE = 1000;
public:
  using ResponseFunction = std::function<void(std::string)>;
  using Ptr = std::shared_ptr<TraceSession>;

  /**
   * @brief Creates the tracing session.
   * @param response_function The function that can be used to send text strings over the websocket.
   */
  TraceSession(ResponseFunction&& response_function);

  /**
   * @brief Add a data source to this session, in general this is done just after creation before data has come in.
   */
  void addSource(TraceEventSource::Ptr&& source);

  /**
   * @brief Method called by the server whenever data is received on the socket.
   */
  void incoming(const std::string& data);

  ~TraceSession();

private:
  ResponseFunction response_;  //!< The function to be used to send data to the client over the websocket.

  std::vector<TraceEventSource::Ptr> sources_;  //!< The active sources in this session.

  mutable std::mutex incoming_mutex_;  //!< Mutex for list of incoming messages that are pending processing.
  std::list<std::string> incoming_msg_;  //!< List of incoming messages that will be processed by the worker thread.
  /**
   * @brief Function to check whether there are messages pending in incoming.
   * @return true if there are messages pending in incoming.
   */
  bool haveIncoming() const;

  /**
   * @brief Pop one incoming message from the list of pending messages.
   * @return The message that was received.
   * @warning This function should only be called if haveIncoming() returned true, otherwise it may cause a crash.
   */
  std::string popIncoming();

  /**
   * @brief Function that passes the message to the response function and prints it into the terminal.
   * @param msg The message to send to the client over the websocket.
   */
  void outgoing(const std::string& msg);

  std::thread worker_;  //!< Worker thread for this session.
  bool running_ { true };  //!< Bool to quit the worker thread gracefully.

  /**
   * @brief The function executed by the worker thread.
   */
  void loop();

  /**
   * @brief Function used to process invidual messages that came in from the client.
   * @param incoming_msg The message as it was received from the websocket.
   */
  void processMessage(const std::string& incoming_msg);

  /**
   * @brief This sends the provided events in a chunked manner over in the Tracing.dataCollected wrapping.
   * @param events The events to transmit to the client.
   */
  void chunkedTransmit(const std::vector<json>& events);

  /**
   * @brief The frontend requires newlines after every trace event in the Tracing.dataCollected return. This function
   *        converts the vector of entries into the appropriate newline delimited version.
   * @param entries The entries to format.
   */
  static std::string formatEvents(const std::vector<json>& entries);
};

}
#endif  // SCALOPUS_CATAPULT_TRACE_SESSION_H
