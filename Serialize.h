#pragma once

#include <fstream>
#include <iostream>

#include "ComponentIndexing.h"

class World;

class Serialize
{
private:
	template <typename Arg, typename ...Args>
	void _match_(size_t hash, World& world, std::ifstream& iffile);

public:
	Serialize() = default;

	template <typename... Args>
	void serialize(World& world, const char* filename);

	template <typename... Args>
	void deserialize(World& world, const char* filename);
};

#include "Serialize.ipp"