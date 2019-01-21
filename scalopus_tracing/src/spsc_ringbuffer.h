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
#pragma once

#include <atomic>

namespace scalopus
{

/**
 * @brief This is a single producer - single consumer ringbuffer.
 * It is moddelled after boosts' spsc_queue, but then a lot simpler and with move semantics.
 * It is backed by a container of a fixed size, this allows runtime determined sizes through vector, or compile time
 * size through use of an array.
 */
template <typename ContainerType>
class SPSCRingBuffer
{
public:
  using ValueType = typename ContainerType::value_type;  //!< The type of the elements in the ringbuffer.

  /**
   * @brief Construct the ringbuffer.
   * @param container The container that's used internally by the ringbuffer to store the data.
   * @throws std::runtime_error If the container size is zero.
   */
  SPSCRingBuffer(ContainerType&& container) : container_{container}, max_size_{container_.size()}
  {
    if (max_size_ == 0)
    {
      throw std::runtime_error("Container passed to the ring buffer may not be zero length.");
    }
  }

  /**
   * @brief Move a value onto the ringbuffer.
   * @return true if the value was stored, false if the ring buffer was full.
   * @note Only one thread may interact with push, another thread may pop at the same time.
   */
  bool push(ValueType&& v)
  {
    const std::size_t write_index = write_index_.load(std::memory_order_relaxed);
    const std::size_t next = (write_index + 1) % max_size_;

    if (next == read_index_.load(std::memory_order_acquire))
    {
      return false;
    }

    container_[write_index] = std::move(v);  // move it into the container.

    write_index_.store(next, std::memory_order_release);

    return true;
  }

  /**
   * @brief Pop a value from the ringbuffer.
   * @return false if no value could be popped.
   * @note Only one thread may interact with pop, another thread may push at the same time.
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
  std::atomic<std::size_t> write_index_{ 0 };  //!< Current write position.
  std::atomic<std::size_t> read_index_{ 0 };   //!< Current read position.
  ContainerType container_;  //!< Container that holds the ringbuffer values.
  const std::size_t max_size_;  //!< Size of the ringbuffer.
};

}