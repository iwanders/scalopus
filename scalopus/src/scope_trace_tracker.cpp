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
#include <scalopus/internal/scope_trace_tracker.h>

namespace scalopus
{
ScopeTraceTracker::ScopeTraceTracker()
{
}

void ScopeTraceTracker::trackEntryExitName(unsigned int id, std::string name)
{
  // For writing we require a unique lock on the mutex.
  std::unique_lock<decltype(entry_exit_mutex_)> lock(entry_exit_mutex_);
  entry_exit_mapping_[id] = name;
}

std::map<unsigned int, std::string> ScopeTraceTracker::getEntryExitMapping()
{
  std::shared_lock<decltype(entry_exit_mutex_)> lock(entry_exit_mutex_);
  return entry_exit_mapping_;
}

ScopeTraceTracker& ScopeTraceTracker::getInstance()
{
  static ScopeTraceTracker instance;
  return instance;
}

}  // namespace scalopus
