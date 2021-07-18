#pragma once

#include <type_traits>
#include <bitset>

template<typename EnumT, typename = typename std::enable_if_t<std::is_enum<EnumT>::value>>
class EnumFlags
{
public:
    constexpr EnumFlags() = default;
    constexpr EnumFlags(EnumT value) : m_bitset(1 << static_cast<std::size_t>(value)) {}
    constexpr EnumFlags(const EnumFlags& other) : m_bitset(other.m_bitset) {}

    constexpr EnumFlags operator|(const EnumFlags& value) const { EnumFlags result = *this; result.m_bitset |= value.m_bitset; return result; };
    constexpr EnumFlags operator&(const EnumFlags& value) const { EnumFlags result = *this; result.m_bitset &= value.m_bitset; return result; };
    constexpr EnumFlags operator^(const EnumFlags& value) const { EnumFlags result = *this; result.m_bitset ^= value.m_bitset; return result; };
    constexpr EnumFlags& operator|=(const EnumFlags& value) { m_bitset |= value.m_bitset; return *this; };
    constexpr EnumFlags& operator&=(const EnumFlags& value) { m_bitset &= value.m_bitset; return *this; };
    constexpr EnumFlags& operator^=(const EnumFlags& value) { m_bitset ^= value.m_bitset; return *this; };
    constexpr EnumFlags operator!() const { EnumFlags result = *this; result.m_bitset.flip(); return result; }
    constexpr operator bool() const { return m_bitset.any(); }

    constexpr EnumFlags operator|(EnumT value) const { EnumFlags result = *this; result.m_bitset |= 1 << static_cast<std::size_t>(value); return result; }
    constexpr EnumFlags operator&(EnumT value) const { EnumFlags result = *this; result.m_bitset &= 1 << static_cast<std::size_t>(value); return result; }
    constexpr EnumFlags operator^(EnumT value) const { EnumFlags result = *this; result.m_bitset ^= 1 << static_cast<std::size_t>(value); return result; }
    constexpr EnumFlags& operator|=(EnumT value) { m_bitset |= 1 << static_cast<std::size_t>(value); return *this; }
    constexpr EnumFlags& operator&=(EnumT value) { m_bitset &= 1 << static_cast<std::size_t>(value); return *this; }
    constexpr EnumFlags& operator^=(EnumT value) { m_bitset ^= 1 << static_cast<std::size_t>(value); return *this; }

private:
    constexpr static int m_bitCount = std::numeric_limits<typename std::underlying_type<EnumT>::type>::digits;
    std::bitset<m_bitCount> m_bitset;
};

template<typename EnumT, typename = typename std::enable_if_t<std::is_enum<EnumT>::value>::type>
constexpr EnumFlags<EnumT> operator|(EnumT left, EnumT right) { return EnumFlags<EnumT>(left) | right; }

template<typename EnumT, typename = typename std::enable_if_t<std::is_enum<EnumT>::value>::type>
constexpr EnumFlags<EnumT> operator&(EnumT left, EnumT right) { return EnumFlags<EnumT>(left) & right; }

template<typename EnumT, typename = typename std::enable_if_t<std::is_enum<EnumT>::value>::type>
constexpr EnumFlags<EnumT> operator^(EnumT left, EnumT right) { return EnumFlags<EnumT>(left) ^ right; }