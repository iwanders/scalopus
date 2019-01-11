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

#ifndef SCALOPUS_SCOPE_INTERNAL_THREAD_NAME_TRACKER_H
#define SCALOPUS_SCOPE_INTERNAL_THREAD_NAME_TRACKER_H

#include "scalopus_general/map_tracker.h"

namespace scalopus
{
/**
 * @brief A singleton class to keep track of thread names.
 */
class ThreadNameTracker : public MapTracker<unsigned long, std::string>
{
private:
  ThreadNameTracker() = default;

public:
  static ThreadNameTracker& getInstance();

  /**
   * @brief Set the thread name of the calling thread.
   * @param name The name to assign to the calling thread.
   */
  void setCurrentName(const std::string& name);

  /**
   * @brief Update an arbritrary entry in the thread name map. This can be useful if the thread itself cannot use the
   *        macro for whatever reason.
   * @param thread_id The thread id, static_cast<unsigned long>(pthread_self()) in general.
   * @param name The name to assign to the thread by this id.
   */
  void setThreadName(unsigned long thread_id, const std::string& name);
};

}  // namespace scalopus
#endif  // SCALOPUS_SCOPE_INTERNAL_THREAD_NAME_TRACKER_H
