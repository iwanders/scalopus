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
#ifndef SCALOPUS_TRACE_CONFIGURATOR_H
#define SCALOPUS_TRACE_CONFIGURATOR_H

#include <atomic>
#include <map>
#include <memory>
#include <mutex>

namespace scalopus
{
/**
 * @brief A singleton to allow configuration of the tracing, on a per process and per thread basis.
 */
class TraceConfigurator
{
private:
  TraceConfigurator();
  TraceConfigurator(const TraceConfigurator&) = delete;
  TraceConfigurator& operator=(const TraceConfigurator&) = delete;
  TraceConfigurator& operator=(TraceConfigurator&&) = delete;
  TraceConfigurator(const TraceConfigurator&&) = delete;

public:
  using Ptr = std::shared_ptr<TraceConfigurator>;
  using WeakPtr = std::weak_ptr<TraceConfigurator>;
  using AtomicBoolPtr = std::shared_ptr<std::atomic_bool>;

  virtual ~TraceConfigurator() = default;
  /**
   * @brief Static method through which the singleton instance can be retrieved.
   * @return Returns the singleton instance of the ScopeConfigurator object.
   */
  static TraceConfigurator::Ptr getInstance();

  /**
   * @brief Retrieve the atomic boolean for the thread requesting it.
   */
  AtomicBoolPtr getThreadStatePtr();

  /**
   * @brief Retrieve the thread state for the calling process.
   */
  bool getThreadState();

  /**
   * @brief Set the new thread state and return the old state.
   */
  bool setThreadState(bool new_state);

  /**
   * @brief Retrieve the process wide boolean pointer.
   */
  AtomicBoolPtr getProcessStatePtr() const;

  /**
   * @brief Retrieve the process wide boolean.
   */
  bool getProcessState() const;

  /**
   * @brief Set the new process state and return the old state.
   */
  bool setProcessState(bool new_state);

  /**
   * @brief Retrieve the map of thread booleans.
   */
  std::map<unsigned long, AtomicBoolPtr> getThreadMap() const;

private:
  AtomicBoolPtr process_state_;  //!< Process wide enable / disable flag.

  mutable std::mutex threads_map_mutex_;                 //!< Mutex for the threads_enabled_ map.
  std::map<unsigned long, AtomicBoolPtr> thread_state_;  //!< Enable / disable per thread.

  /**
   * @brief Method to be called when a thread is removed.
   */
  void removeThread(unsigned long thread_id);
};
}  // namespace scalopus

#endif  // SCALOPUS_TRACE_CONFIGURATOR_H
