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
#include "scalopus_general/general_source.h"

#include <sstream>

namespace scalopus
{
GeneralSource::GeneralSource(GeneralProvider::Ptr provider) : provider_(provider)
{
}

std::vector<json> GeneralSource::finishInterval()
{
  provider_->updateMapping();

  std::vector<json> result;
  auto mapping = provider_->getMapping();

  // Iterate over all mappings by process ID.
  for (const auto pid_process_info : mapping)
  {
    // make a metadata entry to name a process.
    json process_entry;
    process_entry["tid"] = 0;
    process_entry["ph"] = "M";
    process_entry["name"] = "process_name";
    process_entry["args"] = { { "name", pid_process_info.second.name } };
    process_entry["pid"] = pid_process_info.first;
    result.push_back(process_entry);

    // For all thread mappings, make a metadata entry to name the thread.
    for (const auto thread_mapping : pid_process_info.second.threads)
    {
      json tid_entry;
      tid_entry["tid"] = thread_mapping.first;
      tid_entry["ph"] = "M";
      tid_entry["name"] = "thread_name";
      tid_entry["pid"] = pid_process_info.first;
      tid_entry["args"] = { { "name", thread_mapping.second } };
      result.push_back(tid_entry);
    }
  }

  return result;
}
}  // namespace scalopus
