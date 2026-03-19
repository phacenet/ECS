
/*
template <typename ...Args>
class View
{
private:
	std::tuple<SparseSet<Args>*...> m_storage;
	ComponentStorage* m_smallest_storage = nullptr;

public:
*/

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
	return ((std::get<SparseSet<Args>*>(m_storage))->has(entityID) || ...);
}

template <typename ...Args>
template <typename T>
void View<Args...>::each(T&& lambda)
{
	for (uint32_t entityID : m_smallest_storage->getDense())
	{
		if constexpr (std::is_invocable_v<T, uint32_t, Args&...>)
		{
			if (((std::get<SparseSet<Args>*>(m_storage)->has(entityID)) && ...))
				lambda(entityID, (std::get<SparseSet<Args>*>(m_storage)->get(entityID))...);
		}

		else
		{
			if (((std::get<SparseSet<Args>*>(m_storage)->has(entityID)) && ...))
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