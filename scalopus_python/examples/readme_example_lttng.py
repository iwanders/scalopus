#!/usr/bin/env python

# Copyright (c) 2018-2019, Ivor Wanders
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the author nor the names of contributors may be used to
#   endorse or promote products derived from this software without specific
#   prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
import sys
import time

import scalopus
import scalopus.tracing as tracing


if not scalopus.tracing.have_lttng:
    print("Could not set lttng as backend, lttng support is not built.")
    sys.exit(1)
else:
    # Swap backend to lttng.
    scalopus.tracing.setBackend(scalopus.tracing.lttng)

@tracing.trace_function
def fooBarBuz():
    time.sleep(0.2)

@tracing.trace_function
def c():
    time.sleep(0.2)
    print("  c")
    fooBarBuz()
    time.sleep(0.2)

@tracing.trace_function
def b():
    time.sleep(0.2)
    print(" b")
    c()
    time.sleep(0.2)

@tracing.trace_function
def a():
    print("a")
    time.sleep(0.2)
    b()
    time.sleep(0.2)

if __name__ == "__main__":
    exposer = scalopus.common.DefaultExposer(process_name=sys.argv[0])
    scalopus.general.setThreadName("main")

    while True:
        # fastest, one attribute lookup, name will be 'my_relevant_scope'
        with tracing.trace_section.my_relevant_scope:
            time.sleep(0.1)
            a()
        # Less fast then above, 1 method lookup and one call, allows spaces. Name will be "My Section"
        with tracing.trace_section("My Section"):
            time.sleep(0.1)
            a()
