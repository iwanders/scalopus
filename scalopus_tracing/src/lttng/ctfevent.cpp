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
#include "scalopus_tracing/ctfevent.h"

namespace scalopus
{
CTFEvent::CTFEvent(std::string line)
{
  // Line we are matching is:
  // "[1544361620.739021131] eagle scalopus_scope_id:exit: { cpu_id = 2 }, { vpid = 14897, pthread_id = 139688084124608
  // }, { id = 4144779573 }"

  line_ = line;

  if (line.empty())
  {
    // should we panic here? This shouldn't happen.
    return;
  }

  // Use substrings and indices for speed...

  // Extract the timestamp, its between []
  size_t start_pos = line.find("[");
  size_t end_pos = line.find("]");
  if (start_pos == std::string::npos || end_pos == std::string::npos)
  {
    return;
  }
  std::string timestamp_str = line.substr(start_pos + 1, end_pos - 1);
  try
  {
    timestamp_ = std::stod(timestamp_str);
  }
  catch (const std::invalid_argument&)
  {
    return;
  }

  // Go for the hostname, this is the first thing between spaces.
  start_pos = end_pos;
  start_pos = line.find(" ", start_pos);
  end_pos = line.find(" ", start_pos + 1);
  if (start_pos == std::string::npos || end_pos == std::string::npos)
  {
    return;
  }
  hostname_ = line.substr(start_pos + 1, end_pos - start_pos - 1);

  // Extract the trace point specification, it's the third entry between spaces.
  start_pos = end_pos;
  start_pos = line.find(" ", start_pos);
  end_pos = line.find(" ", start_pos + 1);
  if (start_pos == std::string::npos || end_pos == std::string::npos)
  {
    return;
  }
  // -1 for removal of space, -1 for removal of ':'
  std::string trace_point = line.substr(start_pos + 1, end_pos - start_pos - 1 - 1);

  // Split the tracepoint in domain and name.
  size_t split_point = trace_point.find(":");
  if (split_point == std::string::npos)
  {
    return;
  }
  tracepoint_domain_ = trace_point.substr(0, split_point);
  tracepoint_name_ = trace_point.substr(split_point + 1, trace_point.size() - split_point);

  // Obtain the stream context between the curly braces.
  start_pos = end_pos;
  start_pos = line.find("{", start_pos);
  end_pos = line.find("}", start_pos + 1);
  if (start_pos == std::string::npos || end_pos == std::string::npos)
  {
    return;
  }
  std::string stream_context = line.substr(start_pos + 2, end_pos - start_pos - 1 - 2);

  // Obtain the event context between the curly braces.
  start_pos = end_pos;
  start_pos = line.find("{", start_pos);
  end_pos = line.find("}", start_pos + 1);
  if (start_pos == std::string::npos || end_pos == std::string::npos)
  {
    return;
  }
  std::string event_context = line.substr(start_pos + 2, end_pos - start_pos - 1 - 2);

  // Obtain the trace data between the curly braces.
  start_pos = end_pos;
  start_pos = line.find("{", start_pos);
  end_pos = line.find("}", start_pos + 1);
  if (start_pos == std::string::npos || end_pos == std::string::npos)
  {
    return;
  }
  std::string trace_data = line.substr(start_pos + 2, end_pos - start_pos - 1 - 2);

  // Make a lambda that can convert the syntax between the curly braces (comma separated values) into a map of integers.
  auto processContext = [](std::string csv) -> std::map<std::string, unsigned long long int> {
    std::map<std::string, unsigned long long int> res;

    size_t start = 0;
    size_t end = 0;
    while (end != csv.size())
    {
      size_t potential = csv.find(", ", start);  // delimiter between the key-value entries.
      size_t end_shift = 0;
      if (potential == std::string::npos)
      {
        // didn't find the delimiter, string to use is the entires tring.
        end = csv.size();
        end_shift = 0;
      }
      else
      {
        end = potential;
        end_shift = 2;  // length of ", "
      }
      // grab the 'key = value' string
      std::string kvpair = csv.substr(start, end - start);
      start = end + end_shift;

      if (kvpair.size() <= 4)  // cannot contain one key value pair, it was likely an empty { } for this context.
      {
        break;
      }

      // now, we have to parse the kvpair.
      size_t kvsplit_point = kvpair.find(" = ");
      std::string key = kvpair.substr(0, kvsplit_point);
      std::string value = kvpair.substr(kvsplit_point + 3);  // 3 is length of " = "
      try
      {
        // if value is a string, this will fail, we need to catch that.
        res[key] = std::stoull(value);
      }
      catch (const std::invalid_argument&)
      {
      }
    }
    return res;
  };

  // convert each context into the appropriate map.
  stream_context_ = processContext(stream_context);
  event_context_ = processContext(event_context);
  tracepoint_data_ = processContext(trace_data);
}

double CTFEvent::time() const
{
  return timestamp_;
}

unsigned long long int CTFEvent::pid() const
{
  return event_context_.at("vpid");
}

unsigned long long int CTFEvent::tid() const
{
  return event_context_.at("pthread_id");
}

std::string CTFEvent::domain() const
{
  return tracepoint_domain_;
}

std::string CTFEvent::name() const
{
  return tracepoint_name_;
}

std::string CTFEvent::line() const
{
  return line_;
}

const CTFEvent::IdMap& CTFEvent::streamContext() const
{
  return stream_context_;
}

const CTFEvent::IdMap& CTFEvent::eventContext() const
{
  return event_context_;
}

const CTFEvent::IdMap& CTFEvent::eventData() const
{
  return tracepoint_data_;
}

}  // namespace scalopus
