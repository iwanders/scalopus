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
#include "scalopus_tracing/scope_tracing.h"

int main(int /* argc */, char** /* argv */)
{
  // Execute all public trace point macros.
  // Mostly to ensure they don't collide with each other if used multiple times
  // or create shadowed variables or something.
  TRACE_PRETTY_FUNCTION();
  TRACE_PRETTY_FUNCTION();

  TRACE_SCOPE_RAII("main");
  TRACE_SCOPE_RAII("main");

  TRACE_SCOPE_START("zz");
  TRACE_SCOPE_START("zz");
  TRACE_SCOPE_END("zz");
  TRACE_SCOPE_END("zz");

  // Check if the macro's don't expand to shadowed variables.
  {
    TRACE_PRETTY_FUNCTION();
    TRACE_PRETTY_FUNCTION();

    TRACE_SCOPE_RAII("main");
    TRACE_SCOPE_RAII("main");

    TRACE_SCOPE_START("zz");
    TRACE_SCOPE_START("zz");
    TRACE_SCOPE_END("zz");
    TRACE_SCOPE_END("zz");
  }
  return 0;
}