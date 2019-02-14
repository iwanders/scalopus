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

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

namespace cbor
{
using Data = std::vector<std::uint8_t>;

/**
 * @brief Swap endianness
 */
std::uint16_t fixEndianness(const std::uint16_t in)
{
  auto b = reinterpret_cast<const std::uint8_t*>(&in);
  return static_cast<std::uint16_t>((b[0] << 8) | b[1]);
}

/**
 * @brief Swap endianness, reduces to bswap assembly instruction.
 */
std::uint32_t fixEndianness(const std::uint32_t in)
{
  auto b = reinterpret_cast<const std::uint8_t*>(&in);
  return static_cast<std::uint32_t>((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]);
}

/**
 * @brief Swap endianness, reduces to bswap assembly instruction.
 */
std::uint64_t fixEndianness(const std::uint64_t in)
{
  auto b = reinterpret_cast<const std::uint8_t*>(&in);
  std::uint32_t upper = static_cast<std::uint32_t>(b[0] << 24) | static_cast<std::uint32_t>(b[1] << 16) |
                        static_cast<std::uint32_t>(b[2] << 8) | static_cast<std::uint32_t>(b[3] << 0);
  std::uint32_t lower = static_cast<std::uint32_t>(b[4] << 24) | static_cast<std::uint32_t>(b[5] << 16) |
                        static_cast<std::uint32_t>(b[6] << 8) | static_cast<std::uint32_t>(b[7]);
  return static_cast<std::uint64_t>((static_cast<std::uint64_t>(upper) << (4 * 8)) | lower);
}

std::string hexdump(const Data& d)
{
  std::stringstream ss;
  for (const auto& v : d)
  {
    ss << "" << std::setfill('0') << std::setw(2) << std::hex << int{ v } << " ";
  }
  return ss.str();
}

/**
 * @brief Serializer trait struct
 */
template <typename T>
struct serializer
{
  static std::uint32_t execute(const T& /* value */, Data& /* out */)
  {
    static_assert(std::is_same<T, void>::value, "This type is not supported by the cbor serialization.");
    return 0;
  }
};

/**
 * @brief Entry to the trait system, serializes v into data.
 * @param v The data to serialize.
 * @param data The data vector to serialize into.
 */
template <typename T>
std::size_t serialize(const T& v, Data& data)
{
  return serializer<T>::execute(v, data);
}

/**
 * @brief An object to represent an already serialized compact binary object.
 */
class cbor_object
{
public:
  Data serialized_;
  /**
   * @brief Function to create a cbor_object that represents the value that was passed in during creation.
   */
  template <typename T>
  static cbor_object make(const T& v)
  {
    cbor_object res;
    serialize(v, res.serialized_);
    return res;
  }
};

/**
 * @brief Function to write a 8 bit unsigned int in its shortest form given the major type.
 */
std::size_t serializeInteger(const std::uint8_t major_type, const std::uint8_t v, Data& data)
{
  if (v <= 23)
  {
    data.resize(data.size() + 1);
    data.back() = std::uint8_t(major_type << 5) | v;
    return 1;
  }
  else
  {
    const std::size_t offset = data.size();
    data.resize(offset + 2);
    data[offset] = std::uint8_t(major_type << 5) | 24;
    data[offset + 1] = v;
    return 2;
  }
}

/**
 * @brief Function to write a 16 bit unsigned int in its shortest form given the major type.
 */
std::size_t serializeInteger(const std::uint8_t major_type, const std::uint16_t v, Data& data)
{
  if (v <= std::numeric_limits<std::uint8_t>::max())
  {
    return serializeInteger(major_type, static_cast<std::uint8_t>(v), data);
  }
  const std::size_t offset = data.size();
  data.resize(offset + 2 + 1);
  data[offset] = std::uint8_t(major_type << 5) | 25;
  auto fixed = fixEndianness(v);
  *reinterpret_cast<std::uint16_t*>(&(data[offset + 1])) = fixed;
  return 3;
}

/**
 * @brief Function to write a 32 bit unsigned int in its shortest form given the major type.
 */
std::size_t serializeInteger(const std::uint8_t major_type, const std::uint32_t v, Data& data)
{
  if (v <= std::numeric_limits<std::uint16_t>::max())
  {
    return serializeInteger(major_type, static_cast<std::uint16_t>(v), data);
  }
  const std::size_t offset = data.size();
  data.resize(offset + 4 + 1);
  auto fixed = fixEndianness(v);
  data[offset] = std::uint8_t(major_type << 5) | 26;
  *reinterpret_cast<std::uint32_t*>(&(data[offset + 1])) = fixed;
  return 5;
}

/**
 * @brief Function to write a 64 bit unsigned int in its shortest form given the major type.
 */
std::size_t serializeInteger(const std::uint8_t major_type, const std::uint64_t v, Data& data)
{
  if (v <= std::numeric_limits<std::uint32_t>::max())
  {
    return serializeInteger(major_type, static_cast<std::uint32_t>(v), data);
  }
  const std::size_t offset = data.size();
  data.resize(offset + 8 + 1);
  data[offset] = std::uint8_t(major_type << 5) | 27;
  auto fixed = fixEndianness(v);
  *reinterpret_cast<std::uint64_t*>(&(data[offset + 1])) = fixed;
  return 9;
}

/**
 * Specialization for uint8_t.
 */
template <>
struct serializer<std::uint8_t>
{
  using Type = std::uint8_t;
  static std::size_t execute(const Type& v, Data& data)
  {
    return serializeInteger(0b000, v, data);
  }
};

/**
 * Specialization for uint16_t.
 */
template <>
struct serializer<std::uint16_t>
{
  using Type = std::uint16_t;
  static std::size_t execute(const Type& v, Data& data)
  {
    return serializeInteger(0b000, v, data);
  }
};

/**
 * Specialization for uint32_t.
 */
template <>
struct serializer<std::uint32_t>
{
  using Type = std::uint32_t;
  static std::size_t execute(const Type& v, Data& data)
  {
    return serializeInteger(0b000, v, data);
  }
};

/**
 * Specialization for uint64_t.
 */
template <>
struct serializer<std::uint64_t>
{
  using Type = std::uint64_t;
  static std::size_t execute(const Type& v, Data& data)
  {
    return serializeInteger(0b000, v, data);
  }
};

/**
 * Specialization for std::string.
 */
template <>
struct serializer<std::string>
{
  using Type = std::string;
  static std::size_t execute(const Type& v, Data& data)
  {
    std::size_t addition = serializeInteger(0b011, v.size(), data);
    data.insert(data.end(), v.begin(), v.end());
    return addition + v.size();
  }
};

/**
 * Specialization for cbor_object.
 */
template <>
struct serializer<cbor_object>
{
  using Type = cbor_object;
  static std::size_t execute(const Type& obj, Data& data)
  {
    const auto& v = obj.serialized_;
    data.insert(data.end(), v.begin(), v.end());
    return v.size();
  }
};

/**
 * Specialization for std::vector.
 */
template <typename T>
struct serializer<std::vector<T>>
{
  using Type = std::vector<T>;
  static std::size_t execute(const Type& v, Data& data)
  {
    std::size_t addition = 0;
    addition += serializeInteger(0b100, v.size(), data);
    for (const auto& k : v)
    {
      addition += serialize(k, data);
    }
    return addition;
  }
};

/**
 * Helper for serializing tuples, recurses down through index.
 */
template <size_t index, typename... Ts>
struct serialize_tuple_element
{
  static std::size_t execute(const std::tuple<Ts...>& t, Data& data)
  {
    std::size_t value = serialize(std::get<sizeof...(Ts) - index>(t), data);
    value += serialize_tuple_element<index - 1, Ts...>::execute(t, data);
    return value;
  }
};

/**
 * Recursion terminator at index 0.
 */
template <typename... Ts>
struct serialize_tuple_element<0, Ts...>
{
  static std::size_t execute(const std::tuple<Ts...>& /*t*/, Data& /*data*/)
  {
    return 0;
  }
};

/**
 * Specialization for std::tuple.
 */
template <typename... Ts>
struct serializer<std::tuple<Ts...>>
{
  static std::size_t execute(const std::tuple<Ts...>& v, Data& data)
  {
    std::size_t addition = 0;
    addition += serializeInteger(0b100, sizeof...(Ts), data);
    return addition + serialize_tuple_element<sizeof...(Ts), Ts...>::execute(v, data);
  }
};

/**
 * Specialization for std::pair
 */
template <typename A, typename B>
struct serializer<std::pair<A, B>>
{
  static std::size_t execute(const std::pair<A, B>& v, Data& data)
  {
    std::size_t addition = 0;
    addition += serializeInteger(0b100, uint8_t{ 2 }, data);
    addition += serialize(v.first, data);
    addition += serialize(v.second, data);
    return addition;
  }
};

/**
 * Specialization for std::map.
 */
template <typename KeyType, typename ValueType>
struct serializer<std::map<KeyType, ValueType>>
{
  static std::size_t execute(const std::map<KeyType, ValueType>& v, Data& data)
  {
    std::size_t addition = 0;
    addition += serializeInteger(0b101, v.size(), data);
    for (const auto& k_v : v)
    {
      const auto& key = k_v.first;
      const auto& value = k_v.second;
      addition += serialize(key, data);
      addition += serialize(value, data);
    }
    return addition;
  }
};
}  // namespace cbor
