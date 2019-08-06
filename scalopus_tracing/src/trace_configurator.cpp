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
#include <scalopus_general/destructor_callback.h>
#include <scalopus_tracing/trace_configurator.h>

namespace scalopus
{
TraceConfigurator::TraceConfigurator()
{
  process_state_ = std::make_shared<std::atomic_bool>(true);  // default to enabled.
}

TraceConfigurator::AtomicBoolPtr TraceConfigurator::getThreadStatePtr()
{
  auto tid = static_cast<unsigned long>(pthread_self());

  //  Register a destructor callback such that the thread gets removed from the map when the thread exits.
  auto instance_pointer = getInstance();
  thread_local DestructorCallback cleanup{ [instance = TraceConfigurator::WeakPtr(instance_pointer), tid]() {
    auto ptr = instance.lock();
    if (ptr != nullptr)
    {
      ptr->removeThread(tid);
    }
  } };
  std::lock_guard<decltype(threads_map_mutex_)> lock(threads_map_mutex_);
  auto it = thread_state_.find(tid);
  if (it != thread_state_.end())
  {
    return thread_state_[tid];  // atomic bool already existed.
  }
  else
  {
    auto boolean = std::make_shared<std::atomic_bool>(true);  // default each thread to enabled.
    thread_state_[tid] = boolean;
    return boolean;
  }
  return nullptr;
}
void TraceConfigurator::removeThread(unsigned long thread_id)
{
  std::lock_guard<decltype(threads_map_mutex_)> lock(threads_map_mutex_);
  thread_state_.erase(thread_id);
}

bool TraceConfigurator::getThreadState()
{
  return *getThreadStatePtr();
}

bool TraceConfigurator::setThreadState(bool new_state)
{
  return getThreadStatePtr()->exchange(new_state);
}

TraceConfigurator::AtomicBoolPtr TraceConfigurator::getProcessStatePtr() const
{
  return process_state_;
}

bool TraceConfigurator::getProcessState() const
{
  return *process_state_;
}

bool TraceConfigurator::setProcessState(bool new_state)
{
  return process_state_->exchange(new_state);
}

std::map<unsigned long, TraceConfigurator::AtomicBoolPtr> TraceConfigurator::getThreadMap() const
{
  std::lock_guard<decltype(threads_map_mutex_)> lock(threads_map_mutex_);
  return thread_state_;
}

TraceConfigurator::Ptr TraceConfigurator::getInstance()
{
  // https://stackoverflow.com/questions/8147027/
  // Trick to allow make_shared with a private constructor.
  struct make_shared_enabler : public TraceConfigurator
  {
  };
  static TraceConfigurator::Ptr instance{ std::make_shared<make_shared_enabler>() };
  return instance;
}

}  // namespace scalopus
