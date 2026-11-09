#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace suz
{
	using int8 = std::uint8_t;
	using uint8 = std::uint8_t;
	using int16 = std::int16_t;
	using uint16 = std::uint16_t;
	using int32 = std::int32_t;
	using uint32 = std::uint32_t;
	using int64 = std::int64_t;
	using uint64 = std::uint64_t;

	using byte = std::uint8_t;
	using usize = size_t;
	using intptr = std::intptr_t;
	using uintptr = std::uintptr_t;

	using String = std::string;

	template<typename T>
	using Vector = std::vector<T>;
	template<typename T, size_t Num>
	using Array = std::array<T, Num>;

	template<typename T>
	using Set = std::set<T>;
	template<typename T>
	using UnorderedSet = std::unordered_set<T>;

	template<typename First, typename Second>
	using Pair = std::pair<First, Second>;

	template<typename Key, typename Value>
	using Map = std::map<Key, Value>;
	template<typename Key, typename Value>
	using UnorderedMap = std::unordered_map<Key, Value>;

	template<typename T>
	using UniquePtr = std::unique_ptr<T>;
	template<typename T>
	using SharedPtr = std::shared_ptr<T>;
}