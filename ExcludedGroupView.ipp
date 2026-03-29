
/*
template <typename OwnedTuple, typename FilteredTuple>
class ExcludedGroupView;

template <typename ...Owned, typename ...Filtered>
class ExcludedGroupView<std::tuple<Owned...>, std::tuple<Filtered...>>
{
private:
	Group<std::tuple<Owned...>, std::tuple<Unowned...>>* m_parent = nullptr;
	std::tuple<SparseSet<Filtered>*> m_filteredSets;
	
public:
*/	

//Ctor
template <typename ...Owned, typename ...Unowned, typename... Filtered>
ExcludedGroupView<std::tuple<Owned...>, std::tuple<Unowned...>, std::tuple<Filtered...>>::
ExcludedGroupView(Group<std::tuple<Owned...>, std::tuple<Unowned...>>* parent, std::tuple<SparseSet<Filtered>*...> filteredSets)
{
	m_parent = parent;
	m_filteredSets = filteredSets;
}

template <typename ...Owned, typename ...Unowned, typename... Filtered>
template <typename T>
void ExcludedGroupView<std::tuple<Owned...>, std::tuple<Unowned...>, std::tuple<Filtered...>>::each(T&& func)
{
	auto& vec = (std::get<0>(m_parent->m_ownedSets))->getDense();

	for (uint32_t entityID : vec)
	{
		if (!((std::get<SparseSet<Filtered>*>(m_filteredSets)->has(entityID)) && ...))
			continue;

		//entity + owned + filtered
		if constexpr (std::is_invocable_v<T, uint32_t, Owned&..., Filtered&...>)
			std::forward<T>(func)(entityID, (std::get<SparseSet<Owned>*>(m_parent->m_ownedSets)->get(entityID))...,
				(std::get<SparseSet<Filtered>*>(m_filteredSets)->get(entityID))...);
		//entity + owned
		else if constexpr (std::is_invocable_v<T, uint32_t, Owned&...>)
			std::forward<T>(func)(entityID, (std::get<SparseSet<Owned>*>(m_parent->m_ownedSets)->get(entityID))...);

		//owned + filtered
		else if constexpr (std::is_invocable_v<T, Owned&..., Filtered&...>)
			std::forward<T>(func)((std::get<SparseSet<Owned>*>(m_parent->m_ownedSets)->get(entityID))...,
				(std::get<SparseSet<Filtered>*>(m_filteredSets)->get(entityID))...);
		//owned only
		else if constexpr (std::is_invocable_v<T, Owned&...>)
			std::forward<T>(func)((std::get<SparseSet<Owned>*>(m_parent->m_ownedSets)->get(entityID))...);

		//entity only
		else if constexpr (std::is_invocable_v<T, uint32_t>)
			std::forward<T>(func)(entityID);

		//fail
		else
			static_assert(always_false<T>, "Unsupported callable signature");
	}
}

template <typename ...Owned, typename ...Unowned, typename... Filtered>
bool ExcludedGroupView<std::tuple<Owned...>, std::tuple<Unowned...>, std::tuple<Filtered...>>::contains(uint32_t entityID)
{
	bool inOwned = ((std::get<SparseSet<Owned>*>(m_parent->m_ownedSets))->has(entityID) && ...);
	
	if constexpr (sizeof...(Filtered) > 0)
	{
		bool inFiltered = ((std::get<SparseSet<Filtered>*>(m_filteredSets))->has(entityID) && ...);
		return (inOwned && inFiltered);
	}

	return (inOwned);
}

template <typename ...Owned, typename ...Unowned, typename... Filtered>
template <typename T>
T& ExcludedGroupView<std::tuple<Owned...>, std::tuple<Unowned...>, std::tuple<Filtered...>>::get(uint32_t entityID) //NEED FIX FOR TAGBASE, then need to fix each
{
	using all_types = decltype(std::tuple_cat(std::declval(std::tuple<Owned...>(), std::tuple<Filtered...>())));
	using filtered_tuple = typename Filter<is_not_tag, all_types>::type;

	if constexpr (sizeof...(Filtered) > 0 && has_type<T, std::tuple<Unowned...>>::value)
		return std::get<SparseSet<T>*>(m_filteredSets)->get(entityID);

	else
		return std::get<SparseSet<T>*>(m_parent->m_ownedSets)->get(entityID);
}

template <typename ...Owned, typename ...Unowned, typename... Filtered>
bool ExcludedGroupView<std::tuple<Owned...>, std::tuple<Unowned...>, std::tuple<Filtered...>>::empty()
{
	return m_parent->empty();
}

template <typename ...Owned, typename ...Unowned, typename... Filtered>
size_t ExcludedGroupView<std::tuple<Owned...>, std::tuple<Unowned...>, std::tuple<Filtered...>>::size()
{
	return m_parent->size();
}

template <typename ...Owned, typename ...Unowned, typename... Filtered>
size_t ExcludedGroupView<std::tuple<Owned...>, std::tuple<Unowned...>, std::tuple<Filtered...>>::size(ComputationType type)
{
	if constexpr (sizeof...(Filtered) > 0)
	{
		std::vector<uint32_t> vec = std::get<0>(m_parent->m_ownedSets)->getDense();
		uint32_t intersects = 0;

		for (uint32_t i{ 0 }; i < m_parent->m_len; ++i)
		{
			uint32_t entityID = vec[i];

			//Owned ID not found in Unowned
			if ((std::get<SparseSet<Filtered>*>(m_filteredSets)->has(entityID) && ...))
				++intersects;
		}
		return intersects;
	}
	return m_parent->size();
}


