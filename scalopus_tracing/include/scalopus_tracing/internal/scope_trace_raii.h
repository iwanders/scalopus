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
#ifndef SCALOPUS_TRACING_SCOPE_TRACE_RAII_H
#define SCALOPUS_TRACING_SCOPE_TRACE_RAII_H

#include <scalopus_tracing/internal/scope_tracepoint.h>

namespace scalopus
{
/**
 * @brief RAII Tracepoint that stores the ID in an entry tracepoint and an exit tracepoint once destroyed.
 */
class TraceRAII
{
  unsigned int id_;  //! Storage of the ID of this tracepoint.
public:
  /**
   * @brief Constructor for the RAII tracepoint.
   * @param id A unique id to refence this tracepoint by.
   */
  TraceRAII(const unsigned int id);

  /**
   * @brief Destructor, emits the exit tracepoint.
   */
  ~TraceRAII();
};

}  // namespace scalopus

#endif  // SCALOPUS_TRACING_SCOPE_TRACE_RAII_H
