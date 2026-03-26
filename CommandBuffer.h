#pragma once

#include <vector>
#include <functional>

#include "World.h"

class CommandBuffer
{
private:
	std::vector<std::function<void()>> m_funcs;
	World* m_owner_world = nullptr;

public:
	CommandBuffer(World& world);

	void destroy(uint32_t entityID);

	template <typename T>
	void remove(uint32_t entityID);

	template <typename ...Args>
	void emplace(uint32_t entityID);

	void flush();

};

#include "CommandBuffer.ipp"