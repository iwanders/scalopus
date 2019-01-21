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
#ifndef SCALOPUS_GENERAL_ENDPOINT_PROCESS_INFO_H
#define SCALOPUS_GENERAL_ENDPOINT_PROCESS_INFO_H

#include <scalopus_interface/transport.h>
#include <map>
#include <string>

namespace scalopus
{
/**
 * @brief This endpoint provides the thread names and process name.
 */
class EndpointProcessInfo : public Endpoint
{
public:
  using Ptr = std::shared_ptr<EndpointProcessInfo>;
  constexpr static const char* name = "process_info";

  //! Struct used to represent the data from the endpoint.
  struct ProcessInfo
  {
    std::string name;                              //!< Name of this process.
    std::map<unsigned long, std::string> threads;  //!< Names of the threads in this process.
    unsigned long pid;                             //!< Process id.
  };

  /**
   * @brief Constructor sets the process id.
   */
  EndpointProcessInfo();

  //  ------ Server ------
  /**
   * @brief Provide a name to use for this process.
   */
  void setProcessName(const std::string& name);

  //  ------   Client ------
  /**
   * @brief Return the process info from the endpoint.
   */
  ProcessInfo processInfo();

  // From the endpoint
  std::string getName() const;
  bool handle(Transport& server, const Data& request, Data& response);

private:
  ProcessInfo info_;  //!< The process info.
};

}  // namespace scalopus

#endif  // SCALOPUS_GENERAL_ENDPOINT_PROCESS_INFO_H
