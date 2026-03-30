#pragma once

#include <tuple>

#include "get_t.h"
#include "SparseSet.h"
#include "Group.h"
#include "GroupFlag.h"


template <typename OwnedTuple, typename UnownedTuple, typename FilteredTuple>
class ExcludedGroupView;

template <typename ...Owned, typename ...Unowned, typename ...Filtered>
class ExcludedGroupView<std::tuple<Owned...>, std::tuple<Unowned...>, std::tuple<Filtered...>>
{
private:
	Group<std::tuple<Owned...>, std::tuple<Unowned...>>* m_parent = nullptr;
	std::tuple<SparseSet<Filtered>*...> m_filteredSets;

	template <typename T>
	auto _filter_type_(uint32_t entityID) -> decltype(auto);

public:
	//Ctor
	ExcludedGroupView(Group<std::tuple<Owned...>, std::tuple<Unowned...>>* parent, std::tuple<SparseSet<Filtered>*...> filteredSets);

	template <typename T>
	void each(T&& func);

	bool contains(uint32_t entityID);

	template <typename T>
	T& get(uint32_t entityID);

	bool empty();

	size_t size();

	size_t size(ComputationType type);

};

#include "ExcludedGroupView.ipp"
