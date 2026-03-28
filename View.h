#pragma once

#include "Core.h"
#include "get_t.h"

class World;
class TagBase;

template <typename ...Args>
class View
{
private:
	std::tuple<SparseSet<Args>*...> m_storage;
	ComponentStorage* m_smallest_storage = nullptr;

	void _recalc_();

public:
	explicit View(World& world);

	template <typename T>
	void each(T&& lambda);

	bool contains(uint32_t entityID);

	template <typename T>
	T& get(uint32_t entityID);

	size_t size();
	bool empty();
};

#include "World.h"
#include "View.ipp"