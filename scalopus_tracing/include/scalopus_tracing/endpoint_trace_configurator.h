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
#ifndef SCALOPUS_TRACING_ENDPOINT_TRACE_CONFIGURATOR_H
#define SCALOPUS_TRACING_ENDPOINT_TRACE_CONFIGURATOR_H

#include <scalopus_interface/transport.h>
#include <map>
#include <string>

namespace scalopus
{
/**
 * @brief This endpoint provides the thread names and process name.
 */
class EndpointTraceConfigurator : public Endpoint
{
public:
  using Ptr = std::shared_ptr<EndpointTraceConfigurator>;
  static const char* name;

  struct TraceConfiguration
  {
    bool process_state{ false };                 //!< Are traces for this process enabled?
    bool set_process_state{ false };             //!< Are we setting the process state?

    bool new_thread_state{ false };                 //!< Are traces for new threads enabled?
    bool set_new_thread_state{ false };             //!< Are we setting the new thread state?

    std::map<unsigned long, bool> thread_state;  //!< Thread state, true = tracing enabled.
    bool cmd_success{ false };

    operator bool() const
    {
      return cmd_success;
    }
  };

  /**
   * @brief Constructor for this endpoint.
   */
  EndpointTraceConfigurator() = default;

  //  ------   Client ------
  /**
   * @brief Set trace state, only modifies threads which are present in the map.
   */
  TraceConfiguration setTraceState(const TraceConfiguration& state) const;

  /**
   * @brief Get the current trace state.
   */
  TraceConfiguration getTraceState() const;

  /**
   * @brief Function to create a new instance of this class and assign the transport to it.
   */
  static Ptr factory(const Transport::Ptr& transport);

  // From the endpoint
  std::string getName() const;
  bool handle(Transport& server, const Data& request, Data& response);
};

}  // namespace scalopus

#endif  // SCALOPUS_TRACING_ENDPOINT_TRACE_CONFIGURATOR_H
