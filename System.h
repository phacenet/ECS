#pragma once

#include "Core.h"

class World;

class System
{
public:
	virtual void update(World& world) = 0;
	virtual ~System() = default;
};