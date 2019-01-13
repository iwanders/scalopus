/*
  Copyright (c) 2019, Ivor Wanders
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
#pragma once

#include <atomic>

namespace scalopus
{

// This is modelled after boost's spsc_queue.

template <typename ContainerType>
class SPSCRingBuffer
{
public:
  using ValueType = typename ContainerType::value_type;

  SPSCRingBuffer(ContainerType&& c) : container_{c}, max_size_{container_.size()}
  {
    if (max_size_ == 0)
    {
      throw std::runtime_error("Size of container that was passed to ringbuffer is zero.");
    }
  }

  /**
   * @brief Push a value onto the ringbuffer.
   * @return true if the value was stored, false if the ring buffer was full.
   * @note Only one thread must interact with push.
   */
  bool push(ValueType&& v)
  {
    const std::size_t write_index = write_index_.load(std::memory_order_relaxed);
    const std::size_t next = (write_index + 1) % max_size_;

    if (next == read_index_.load(std::memory_order_acquire))
    {
      return false;
    }

    container_[write_index] = std::move(v);  // assign it into the container.

    write_index_.store(next, std::memory_order_release);

    return true;
  }

  /**
   * @brief Pop a value from the ringbuffer.
   * @return false if no value could be popped.
   * @note Only one thread must interact with pop.
   */
  bool pop(ValueType& v)
  {
    const std::size_t write_index = write_index_.load(std::memory_order_acquire);
    const std::size_t read_index  = read_index_.load(std::memory_order_relaxed);

    if (write_index == read_index)
    {
      return false;
    }

    v = std::move(container_[read_index]);  // move the value from the container into the value type.

    std::size_t next = (read_index + 1) % max_size_;
    read_index_.store(next, std::memory_order_release);
    return true;
  }

  /**
   * @brief Return whether the ringbuffer currently is empty.
   */
  bool empty() const
  {
    return write_index_.load(std::memory_order_relaxed) == read_index_.load(std::memory_order_relaxed);
  }

private:
  std::atomic<std::size_t> write_index_{ 0 };
  std::atomic<std::size_t> read_index_{ 0 };
  ContainerType container_;
  const std::size_t max_size_;
};

}