#pragma once

#include "ObserverBase.h"
#include "Group.h"


template <typename OwnedTuple, typename UnownedTuple>
class GroupObserver;


template <typename... Owned, typename ...Unowned>
class GroupObserver<std::tuple<Owned...>, std::tuple<Unowned...>> : public ObserverBase
{
private:
	Group<std::tuple<Owned...>, std::tuple<Unowned...>>* m_group_ptr;

public:
	/* Ctor */
	GroupObserver(); //required for storing a GroupObserver by value in Group

	GroupObserver(Group<std::tuple<Owned...>, std::tuple<Unowned...>>& group);

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
