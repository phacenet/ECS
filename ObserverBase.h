#pragma once


class ObserverBase
{
public:
	virtual void notifyAdd(uint32_t entityID) = 0;
	virtual void notifyRemove(uint32_t entityID) = 0;
	virtual void clear() = 0;
	virtual ~ObserverBase() = default;
};
