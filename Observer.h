#pragma once

#include "Core.h"

#include "ObserverBase.h"
#include "World.h"

/* TODO: Make matching entities container do something: pattern is when an entity gets the matching component after the observer was created, add the id to the matching list
ONLY cleared when the user clears it. Otherwise just keeps accumulating. Also need a debug to view observers, etc. Then groups, and Ill be back to where I was. */


class Observer : public ObserverBase
{
private:
	World* m_world_ptr = nullptr;
	std::vector<uint32_t> m_matchingEntities;
	std::vector<ComponentStorage*> m_monitoredComponents;

public:
	/* Ctor */
	Observer(World& world);

	template <typename T>
	void observeAdd();

	template <typename T>
	void observeRemove();

	template <typename T>
	void each(T&& lambda);

	size_t size();

	virtual void clear();

	enum class ObserverType : uint8_t { ONADD, ONREMOVE, BOTH, SAFE };
	template <typename T>
	void unregister(ObserverType type = ObserverType::SAFE);

	void unregisterAll();

	~Observer()
	{
		unregisterAll();
	}
	
private:
	//Helper for repeated check
	void checkIfMonitored(ComponentStorage* cs);
	virtual void notifyAdd(uint32_t entityID) override;
	virtual void notifyRemove(uint32_t entityID) override;

};

#include "Observer.ipp"