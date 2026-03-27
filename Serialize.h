#pragma once

#include <fstream>

#include "World.h"


class Serialize
{
private:

public:
	Serialize() = default;

	template <typename... Args>
	void serialize(World& world, const char* filename);

	template <typename... Args>
	void deserialize(World& world, const char* filename);
};

#include "Serialize.ipp"