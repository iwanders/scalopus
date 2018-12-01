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

#ifndef SCALOPUS_SCOPE_TRACE_TRACKER_H
#define SCALOPUS_SCOPE_TRACE_TRACKER_H

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
class TraceTracker
{
public:
  TraceTracker(TraceTracker const&) = delete;    //! Delete the copy constructor.
  void operator=(TraceTracker const&) = delete;  //! Delete the assigment operator.

  /**
   * @brief Static method through which the singleton instance can be retrieved.
   * @return Returns the singleton instance of the TraceTracker object.
   */
  static TraceTracker& getInstance();

  /**
   * @brief Function to insert an id->name mapping. Is thread safe.
   * @param id The id to look up in the map, if it is already present nothing is done, if it isn't present it is added.
   * @param name The name corresponding to the id.
   */
  void trackEntryExitName(unsigned int id, std::string name);

  /**
   * @brief Function to retrieve the mapping between entry and exit ids and provided names.
   * @return Map that holds trace point id - trace name mapping.
   */
  std::map<unsigned int, std::string> getEntryExitMapping();

private:
  std::map<unsigned int, std::string> entry_exit_mapping_;  //! The map in which the id's and names are stored.
  std::shared_timed_mutex entry_exit_mutex_;                //! Mutex for the mapping container.

  TraceTracker();  //! Make constructor private such that we can ensure it is a singleton.
};
}  // namespace scalopus

#endif  // SCALOPUS_SCOPE_TRACE_TRACKER_H
