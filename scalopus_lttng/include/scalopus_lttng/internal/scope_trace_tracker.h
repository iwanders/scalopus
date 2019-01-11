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

#ifndef SCALOPUS_LTTNG_SCOPE_TRACE_TRACKER_H
#define SCALOPUS_LTTNG_SCOPE_TRACE_TRACKER_H

#include <scalopus_general/map_tracker.h>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>

namespace scalopus
{
/**
 * @brief A singleton class that keeps track of the mapping between the ID's stored in the trace and the user-provided
          name for them.
 */
class ScopeTraceTracker : public MapTracker<unsigned int, std::string>
{
private:
  ScopeTraceTracker() = default;

public:
  /**
   * @brief Static method through which the singleton instance can be retrieved.
   * @return Returns the singleton instance of the ScopeTraceTracker object.
   */
  static ScopeTraceTracker& getInstance();
};
}  // namespace scalopus

#endif  // SCALOPUS_LTTNG_SCOPE_TRACE_TRACKER_H
