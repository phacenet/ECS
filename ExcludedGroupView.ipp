
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

	using all_types = decltype(std::tuple_cat(std::declval<std::tuple<Owned...>>(), std::declval<std::tuple<Filtered...>>()));
	using owned_types = typename Filter<is_not_tag, std::tuple<Owned...>>::type;
	using all_filtered = typename Filter<is_not_tag, all_types>::type;

	for (uint32_t entityID : vec)
	{
		if (!((std::get<SparseSet<Filtered>*>(m_filteredSets)->has(entityID)) && ...))
			continue;

		auto result = std::tuple_cat(_filter_type_<Owned>(entityID)..., _filter_type_<Filtered>(entityID)...);
		auto owned_result = std::tuple_cat(_filter_type_<Owned>(entityID)...);

		//entity + owned + filtered
		if constexpr (is_invocable_with_tuple_and_id<T, uint32_t, all_filtered>::value)
			std::apply([&](auto&... args) {func(entityID, args...); }, result);

		//entity + owned
		else if constexpr (is_invocable_with_tuple_and_id<T, uint32_t, owned_types>::value)
			std::apply([&](auto&... args) {func(entityID, args...); }, owned_result);

		//owned + filtered
		else if constexpr (is_invocable_with_tuple<T, all_filtered>::value)
			std::apply([&](auto&... args) {func(args...); }, result);

		//owned only
		else if constexpr (is_invocable_with_tuple<T, owned_types>::value)
			std::apply([&](auto&... args) {func(args...); }, owned_result);

		//entity only
		else if constexpr (is_invocable_with_id<T>::value)
			func(entityID);

		//fail
		else
			static_assert(always_false<T>, "Unsupported callable signature");
	}
}

//private helper for each
template <typename... Owned, typename...Unowned, typename... Filtered>
template <typename T>
auto ExcludedGroupView<std::tuple<Owned...>, std::tuple<Unowned...>, std::tuple<Filtered...>>::_filter_type_(uint32_t entityID) -> decltype(auto)
{
	if constexpr (std::is_base_of_v<TagBase, T>)
		return std::tuple<>{};

	else if constexpr ((std::is_same_v<T, Owned> || ...))
		return std::forward_as_tuple(std::get<SparseSet<T>*>(m_parent->m_ownedSets)->get(entityID));

	else if constexpr ((std::is_same_v<T, Filtered> || ...))
		return std::forward_as_tuple(std::get<SparseSet<T>*>(m_filteredSets)->get(entityID));

	else
		static_assert(always_false<T>, "Type not found in Owned... or Filtered...");
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
T& ExcludedGroupView<std::tuple<Owned...>, std::tuple<Unowned...>, std::tuple<Filtered...>>::get(uint32_t entityID)
{
	static_assert(!std::is_base_of_v<TagBase, T>, "Classes inheriting from TagBase have no Data to fetch");

	if constexpr (sizeof...(Filtered) > 0 && has_type<T, std::tuple<Filtered...>>::value)
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


