#pragma once

#include <cstdint>
#include <string_view>


/* If a free function is inside a header and not templated or in a class/struct, must be marked inline */

inline uint32_t nextIndex()
{
	static uint32_t counter = 0;
	return counter++; //intentional post-increment, return counter THEN increment
}

template <typename T>
uint32_t getTypeIndex()
{
	static uint32_t index = nextIndex(); //only ever runs once. static "=" only executes once ever
	return index;
}


// For serialization to match types at compile-time
/* Hashing adjusted from
	* https://stackoverflow.com/questions/9385782/fnv-hashing-for-strings-in-c
	* https://stackoverflow.com/questions/48896142/is-it-possible-to-get-hash-values-as-compile-time-constants
*/
template <typename Str>
constexpr size_t hashString(const Str& toHash)
{
	size_t result;

	//64x
	if constexpr (sizeof(size_t) == 8)
	{
		result = 14695981039346656037; //FNV offset basis
		for (char c : toHash)
		{
			result ^= c;
			result *= 1099511628211;
		}
	}

	//x86
	else
	{
		result = 2166136261; //FNV offset basis
		for (char c : toHash)
		{
			result ^= c;
			result *= 16777619;
		}
	}

	return result;
}

template <typename T>
constexpr std::string_view get_type_name()
{
	//MSVC
	#ifdef __FUNCSIG__
	std::string_view name = __FUNCSIG__;

	std::string_view prefix = "get_type_name<";
	std::string_view suffix = ">";

	auto start = name.find(prefix) + prefix.length();
	auto end = name.rfind(suffix);

	return name.substr(start, end - start);
	#endif

	// GCC / clang
	#ifdef __PRETTY_FUNCTION__
	std::string_view name = __PRETTY_FUNCTION__;

	std::string_view prefix = "[T = ";
	std::string_view suffix = "]";

	auto start = name.find(prefix) + prefix.length();
	auto end = name.find(suffix, start);

	return name.substr(start, end - start);
	#endif
}

template <typename T>
constexpr size_t getHash()
{
	return hashString(get_type_name<T>());
}



template <auto Function>
const void* getFunctionID()
{
	static const int sentinel = 0;
	return &sentinel;
}
