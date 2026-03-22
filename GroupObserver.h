#pragma once

#include "ObserverBase.h"
#include "Group.h"

template <typename ...Args>
class GroupObserver : public ObserverBase
{
private:
	Group<Args...>* m_group_ptr;
	std::vector<ComponentStorage*> m_monitoredComponents;

public:
	/* Ctor */
	GroupObserver(Group<Args...>& group);

	template <typename T>
	void observeAdd();

	template <typename T>
	void observeRemove();

	template <typename T>
	void each(T&& lambda);

	enum class ObserverType : uint8_t { ONADD, ONREMOVE, BOTH, SAFE };
	template <typename T>
	void unregister(ObserverType type = ObserverType::BOTH);

	void unregisterAll();

	virtual void clear();
	virtual void notifyAdd(uint32_t entityID) override;
	virtual void notifyRemove(uint32_t entityID) override;

	~GroupObserver()
	{
		unregisterAll();
	}

private:
	//Helper for repeated check
	void checkIfMonitored(ComponentStorage* cs);

};

#include "GroupObserver.ipp"