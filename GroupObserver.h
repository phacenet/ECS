#pragma once

#include "ObserverBase.h"
#include "World.h"

class GroupObserver : public ObserverBase
{
private:
	World* m_world_ptr = nullptr;

public:
	/* Ctor */
	GroupObserver(World& world);

	template <typename T>
	void observeAdd();

	template <typename T>
	void observeRemove();

	template <typename T>
	void each(T&& lambda);

	size_t size();

	virtual void clear();
	virtual void notifyAdd(uint32_t entityID) override;
	virtual void notifyRemove(uint32_t entityID) override;



};

#include "Observer.ipp"