#pragma once

#include "ObserverBase.h"
#include "Group.h"

template <typename ...Args>
class GroupObserver : public ObserverBase
{
private:
	Group<Args...>* m_group_ptr;

public:
	/* Ctor */
	GroupObserver(); //required for storing a GroupObserver by value in Group
	GroupObserver(Group<Args...>& group);

	template <typename T>
	void observeAdd();

	template <typename T>
	void observeRemove();

	void unregisterAll();

	virtual void clear();

	~GroupObserver()
	{
		unregisterAll();
	}

private:
	virtual void notifyAdd(uint32_t entityID) override;
	virtual void notifyRemove(uint32_t entityID) override;
};

#include "GroupObserver.ipp"