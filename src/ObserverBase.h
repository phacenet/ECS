#pragma once

template <typename T>
class SparseSet;

class ObserverBase
{
private:
	virtual void notifyAdd(uint32_t entityID) = 0;
	virtual void notifyRemove(uint32_t entityID) = 0;

public:
	virtual void clear() = 0;

	virtual ~ObserverBase() = default;

	template <typename T>
	friend class SparseSet;
};
