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

#include <memory>
#include <iostream>
#include <cstdint>
#include <vector>
#include <tuple>
#include <iostream>
#include <iomanip>
#include <limits>
#include <map>

namespace cbor
{

  std::uint16_t fixEndianness(const std::uint16_t in)
  {
    auto b = reinterpret_cast<const std::uint8_t*>(&in);
    return static_cast<std::uint16_t>((b[1]<< 8) | b[0]);
  }

  std::uint32_t fixEndianness(const std::uint32_t in)
  {
    auto b = reinterpret_cast<const std::uint8_t*>(&in);
    return static_cast<std::uint32_t>((b[0]<< 24) | (b[1]<< 16) | (b[2]<< 8) | b[3]);
  }

  std::uint64_t fixEndianness(const std::uint64_t in)
  {
    auto b = reinterpret_cast<const std::uint8_t*>(&in);
    std::uint32_t upper = static_cast<std::uint32_t>(b[0]<< 24) | static_cast<std::uint32_t>(b[1]<< 16) | static_cast<std::uint32_t>(b[2]<< 8) | static_cast<std::uint32_t>(b[3]<< 0);
    std::uint32_t lower = static_cast<std::uint32_t>(b[4]<< 24) | static_cast<std::uint32_t>(b[5]<< 16) | static_cast<std::uint32_t>(b[6]<< 8) | static_cast<std::uint32_t>(b[7]);
    return static_cast<std::uint64_t>((static_cast<std::uint64_t>(upper) << (4 * 8))| lower);
  }

  std::string hexdump(const std::vector<std::uint8_t>& d)
  {
    std::stringstream ss;
    for (const auto& v : d)
    {
      ss << "0x" << std::setfill('0') << std::setw(2) << std::hex << int{v} << ", " ;
    }
    return ss.str();
  }

  template <typename T>
  struct serializer
  {
    static std::uint32_t serialize()
    {
      std::cout << "void" << std::endl;
      return 0;
    }
  };

  using Data = std::vector<std::uint8_t>;  
  class cbor_object
  {
    public:
      Data serialized_;
      template <typename T>
      static cbor_object make(const T& v)
      {
        cbor_object res;
        serializer<T>::serialize(v, res.serialized_);
        return res;
      }
  };



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


  template <typename T>
  std::size_t serialize(const T& v, Data& data)
  {
    return serializer<T>::serialize(v, data);
  }

  template <typename T>
  cbor_object to_cbor(const T& v)
  {
    return cbor_object::make(v);
  }

  template <>
  struct serializer<std::uint8_t>
  {
    using Type = std::uint8_t;
    static std::size_t serialize(const Type& v, Data& data)
    {
      return serializeInteger(0b000, v, data);
    }
  };

  template <>
  struct serializer<std::uint16_t>
  {
    using Type = std::uint16_t;
    static std::size_t serialize(const Type& v, Data& data)
    {
      return serializeInteger(0b000, v, data);
    }
  };

  template <>
  struct serializer<std::uint32_t>
  {
    using Type = std::uint32_t;
    static std::size_t serialize(const Type& v, Data& data)
    {
      return serializeInteger(0b000, v, data);
    }
  };

  template <>
  struct serializer<std::uint64_t>
  {
    using Type = std::uint64_t;
    static std::size_t serialize(const Type& v, Data& data)
    {
      return serializeInteger(0b000, v, data);
    }
  };

  template <>
  struct serializer<std::string>
  {
    using Type = std::string;
    static std::size_t serialize(const Type& v, Data& data)
    {
      std::size_t addition = serializeInteger(0b011, v.size(), data);
      std::size_t str_start = data.size();
      data.insert(data.begin() + static_cast<long>(str_start), v.begin(), v.end());
      //  data.resize(data.size() + 1);
      //  data.back() = 0;
      return addition + v.size();
    }
  };

  template <>
  struct serializer<cbor_object>
  {
    using Type = cbor_object;
    static std::size_t serialize(const Type& obj, Data& data)
    {
      const auto& v = obj.serialized_;
      std::size_t str_start = data.size();
      data.insert(data.begin() + static_cast<long>(str_start), v.begin(), v.end());
      return v.size();
    }
  };

  template <typename T>
  struct serializer<std::vector<T>>
  {
    using Type = std::vector<T>;
    static std::size_t serialize(const Type& v, Data& data)
    {
      std::size_t addition = 0;
      addition += serializeInteger(0b100, v.size(), data);
      for (const auto& k : v)
      {
        addition += serializer<typename Type::value_type>::serialize(k, data);
      }
      return addition;
    }
  };

  template<size_t index, typename... Ts>
  struct serialize_tuple_element
  {
    static std::size_t serialize(const std::tuple<Ts...>& t, Data& data)
    {
      using ElementType = typename std::tuple_element<sizeof...(Ts) - index - 1, std::tuple<Ts...>>::type ;
      std::size_t value = serializer< ElementType >::serialize(std::get<sizeof...(Ts) - index - 1>(t), data);
      value += serialize_tuple_element<index - 1, Ts...>::serialize(t, data);
      return value;
    }
  };

   
  template<typename... Ts>
  struct serialize_tuple_element<0, Ts...>
  {
    static std::size_t serialize(const std::tuple<Ts...>& t, Data& data)
    {
      using ElementType = typename std::tuple_element<0, std::tuple<Ts...>>::type ;
      return serializer< ElementType >::serialize(std::get<sizeof...(Ts) - 1>(t), data);
    }
  };

  template<typename... Ts>
  struct serializer<std::tuple<Ts...>>
  {
    static std::size_t serialize(const std::tuple<Ts...>& v, Data& data)
    {
      std::size_t addition = 0;
      addition += serializeInteger(0b100, sizeof...(Ts), data);
      return addition + serialize_tuple_element<sizeof...(Ts)-1, Ts...>::serialize(v, data);
    }
  };

  template <typename KeyType, typename ValueType>
  struct serializer<std::map<KeyType, ValueType>>
  {
    static std::size_t serialize(const std::map<KeyType, ValueType>& v, Data& data)
    {
      std::size_t addition = 0;
      addition += serializeInteger(0b101, v.size(), data); // should be this :(
      for (const auto& k_v : v)
      {
        const auto& key = k_v.first;
        const auto& value = k_v.second;
        addition += serializer<KeyType>::serialize(key, data);
        addition += serializer<ValueType>::serialize(value, data);
      }
      return addition;
    }
  };
}  // namespace cbor
