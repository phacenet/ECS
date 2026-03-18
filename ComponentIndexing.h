#pragma once

#include <cstdint>

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
