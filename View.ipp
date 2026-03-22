
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
		/*
			* MUST evaluate entire fold first or a nasty bug where get is called on an OOB index can occur
			Compiler can evaluate RHS of && even if left side is false, because order is unspecified
			* Also recalc m_smallest_storage to reflect changes since view was captured. Entity could
				have gained or lost Components since inception
		*/
		bool all_have = (std::get<SparseSet<Args>*>(m_storage)->has(entityID) && ...);

		if constexpr (std::is_invocable_v<T, uint32_t, Args&...>)
		{
			if (all_have)
				lambda(entityID, (std::get<SparseSet<Args>*>(m_storage)->get(entityID))...);
		}
		else if constexpr (std::is_invocable_v<T, uint32_t>)
		{
			if(all_have)
				lambda(entityID);
		}
		else
		{
			if (all_have)
				lambda((std::get<SparseSet<Args>*>(m_storage)->get(entityID))...);
		}
	}
}

//User is responsible for ensuring entityID is inside view
template <typename ...Args>
template <typename T>
T& View<Args...>::get(uint32_t entityID)
{
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

//add more functionality to debug functions and make sure view is working properly, then do groups