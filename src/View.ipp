
/*
template <typename ...Args>
class View
{
private:
	std::tuple<SparseSet<Args>*...> m_storage;
	ComponentStorage* m_smallest_storage = nullptr;

public:
*/

//Private helper to recalc m_smallest_storage
template <typename ...Args>
void View<Args...>::_recalc_()
{
	auto tmp = std::apply([](auto&&... ptrs) {return std::array<ComponentStorage*, sizeof...(ptrs)>{ptrs...}; }, m_storage);

	auto comp = [&](ComponentStorage* cmp1, ComponentStorage* cmp2) -> bool
		{
			return (cmp1->getDenseSize() < cmp2->getDenseSize());
		};

	auto it = std::min_element(tmp.begin(), tmp.end(), comp);
	m_smallest_storage = *it;
}

//Ctor
template <typename ...Args>
View<Args...>::View(World& world)
{
	m_storage = { static_cast<SparseSet<Args>*>(world.getComponent(getTypeIndex<Args>()))... };

	std::array<ComponentStorage*, sizeof...(Args)> tmp;
	tmp = { (world.getComponent(getTypeIndex<Args>()))... };

	auto comp = [&](ComponentStorage* cmp1, ComponentStorage* cmp2) -> bool
		{
			return (cmp1->getDenseSize() < cmp2->getDenseSize());
		};

	auto it = std::min_element(tmp.begin(), tmp.end(), comp);
	m_smallest_storage = *it;
}

template <typename ...Args>
bool View<Args...>::contains(uint32_t entityID)
{
	return ((std::get<SparseSet<Args>*>(m_storage))->has(entityID) && ...);
}

template <typename ...Args>
template <typename T>
void View<Args...>::each(T&& lambda)
{
	_recalc_();
	for (uint32_t entityID : m_smallest_storage->getDense())
	{
		bool all_have = (std::get<SparseSet<Args>*>(m_storage)->has(entityID) && ...);

		auto filterLambda = [this, entityID]<typename Arg>() ->decltype(auto)
		{
			if constexpr (std::is_base_of_v<TagBase, Arg>)
				return std::tuple<>{};
			else
				return std::forward_as_tuple(std::get<SparseSet<Arg>*>(m_storage)->get(entityID));
		};
		auto result = std::tuple_cat(filterLambda.template operator()<Args>()...);

		using filtered_tuple = typename Filter <is_not_tag, std::tuple<Args...>>::type;

		if (all_have)
		{
			if constexpr (is_invocable_with_tuple_and_id<T, uint32_t, filtered_tuple>::value)
			{
				std::apply([&](auto&... args) {lambda(entityID, args...); }, result);
				//lambda(entityID, (std::get<SparseSet<Args>*>(m_storage)->get(entityID))...);
			}

			else if constexpr (is_invocable_with_tuple<T, filtered_tuple>::value)
			{
				std::apply([&](auto&... args) {lambda(args...); }, result);
			}

			else if constexpr (is_invocable_with_id<T>::value)
				lambda(entityID);

			else
				static_assert(always_false<T>, "Unsupported callable signature"); //do not pass a TagBase in the lambda
		}
	}
}

//User is responsible for ensuring entityID is inside view
template <typename ...Args>
template <typename T>
T& View<Args...>::get(uint32_t entityID)
{
	static_assert(is_not_tag<T>::value, "Cannot get data for class inheriting from TagBase");
	return std::get<SparseSet<T>*>(m_storage)->get(entityID);
}

// Size of smallest storage's dense array
template <typename ...Args>
size_t View<Args...>::size()
{
	return m_smallest_storage->getDenseSize();
}

template <typename ...Args>
bool View<Args...>::empty()
{
	return (m_smallest_storage->getDenseSize() == 0);
}

