#pragma once

#include <cstdint>
#include <vector>
#include <typeindex>

/* Can remove and check if entityID is in SparseSet without downcasting */
class ComponentStorage
{
public:
	virtual bool remove(uint32_t entityID) = 0;
	virtual bool has(uint32_t entityID) = 0;

	virtual const std::vector<uint32_t>& getDense() = 0;
	virtual const std::vector<uint32_t>& getSparse() = 0;
	virtual std::vector<uint32_t>& getDenseMutable() = 0;
	virtual std::vector<uint32_t>& getSparseMutable() = 0;

	virtual void swapData(uint32_t index1, uint32_t index2) = 0;

	virtual std::type_index getType() = 0;
	virtual size_t getDenseSize() = 0;
	virtual ~ComponentStorage() = default;
};