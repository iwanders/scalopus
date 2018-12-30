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

#ifndef SCALOPUS_LTTNG_CTFEVENT_H
#define SCALOPUS_LTTNG_CTFEVENT_H

#include <map>
#include <ostream>
#include <string>

namespace scalopus
{
/**
 * @brief Class to represent a common trace format event.
 * @note Can only handle unsigned long long integer types in the stream, event and tracepoint data.
 */
class CTFEvent
{
public:
  using IdMap = std::map<std::string, unsigned long long int>;
  /**
   * @brief Populate the CTFEvent from a babeltrace line string.
   * @param line to parse and populate the object from.
   */
  CTFEvent(std::string line);
  CTFEvent() = default;

  /**
   * @brief Timestamp of this event.
   * @return time in seconds since unix epoch.
   */
  double time() const;

  /**
   * @brief Process id this tracepoint originated from.
   * @return the pid this tracepoint was emitted from.
   */
  unsigned long long int pid() const;

  /**
   * @brief The thread id this tracepoint originated from.
   * @return The value of pthread_id of the thread that created the tracepoint.
   */
  unsigned long long int tid() const;

  /**
   * @brief The tracepoint domain.
   * @return The domain of this tracepoint.
   */
  std::string domain() const;

  /**
   * @brief The tracepoint name.
   * @return The name of this tracepoint.
   */
  std::string name() const;

  /**
   * @brief The line used to create this tracepoint from.
   * @return The original line representing the tracepoint as produced by babeltrace.
   */
  std::string line() const;

  const IdMap& streamContext() const;
  const IdMap& eventContext() const;
  const IdMap& eventData() const;

private:
  std::string line_;  //! Original line from babeltrace.

  double timestamp_;               //! Timestamp of this event.
  std::string hostname_;           //! Hostname this event originated from.
  std::string tracepoint_domain_;  //! Tracepoint domain of this event.
  std::string tracepoint_name_;    //! Tracepoint name of this event.

  IdMap stream_context_;   //! Map of the integers in the stream context.
  IdMap event_context_;    //! Map of the integers in the event context.
  IdMap tracepoint_data_;  //! Map of the integers in the event data.

public:
  /**
   * @brief Conversion operator to allow printing this event to std::cout and other ostream types.
   * @param event The event to append to the string in string representation.
   */
  friend inline std::ostream& operator<<(std::ostream& stream, const CTFEvent& event)
  {
    stream << "Event " << event.timestamp_ << " : " << event.tracepoint_domain_ << ":" << event.tracepoint_name_;
    if (!event.stream_context_.empty())
    {
      stream << " stream_context{";
      for (const auto& it : event.stream_context_)
      {
        stream << " " << it.first << "=" << it.second << ", ";
      }
      stream << "}";
    }
    if (!event.stream_context_.empty())
    {
      stream << " event_context{";
      for (const auto& it : event.event_context_)
      {
        stream << " " << it.first << "=" << it.second << ", ";
      }
      stream << "}";
    }
    if (!event.tracepoint_data_.empty())
    {
      stream << " tracepoint_data{";
      for (const auto& it : event.tracepoint_data_)
      {
        stream << " " << it.first << "=" << it.second << ", ";
      }
      stream << "}";
    }
    return stream;
  }
};

}  // namespace scalopus

#endif
