#pragma once

#include <fstream>
#include <cstdint>
#include <vector>
#include <typeindex>

class ObserverBase;

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
	virtual uint32_t getDenseIndex(uint32_t entityID) = 0;
	virtual uint32_t getSparseIndex(uint32_t denseIndex) = 0;

	virtual void addObserverOnAdd(ObserverBase* obs) = 0;
	virtual void addObserverOnRemove(ObserverBase* obs) = 0;

	virtual void remove_observerOnAdd(ObserverBase* obs) = 0;
	virtual void remove_observerOnRemove(ObserverBase* obs) = 0;
	virtual void remove_observerBoth(ObserverBase* obs) = 0;

	virtual void serialize(std::ofstream& outFile) = 0;
	virtual void deserialize(std::ifstream& inFile) = 0;

	virtual std::type_index getType() = 0;
	virtual size_t getDenseSize() = 0;
	virtual ~ComponentStorage() = default;
};