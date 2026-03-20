#pragma once

#include "Core.h"

/* Compute hash of each type_index in vector, and perform bitwise shift and XOR */
struct vector_typeIndex_hash
{
	std::size_t operator()(const std::vector<std::type_index>& vec) const noexcept
	{
		std::size_t seed = vec.size();

		for (auto& e : vec)
		{
			std::size_t h = std::hash<std::type_index>{}(e);
			seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2); //Formula from https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
		}
		return seed;
	}
};