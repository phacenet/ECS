

/*
class GroupObserver : public ObserverBase
{
private:
	Group<Args...>* m_group_ptr;

	* Group has:
		std::tuple<SparseSet<Args>*...> m_ownedSets;
		uint32_t m_len;

public:
*/

//Default Ctor
template <typename ...Owned, typename ...Unowned>
GroupObserver<std::tuple<Owned...>, std::tuple<Unowned...>>::GroupObserver()
	:m_group_ptr(nullptr) {}


//Ctor
template <typename ...Owned, typename ...Unowned>
GroupObserver<std::tuple<Owned...>, std::tuple<Unowned...>>::GroupObserver(Group<std::tuple<Owned...>, std::tuple<Unowned...>>& group)
	: m_group_ptr(&group) {}


template <typename ...Owned, typename ...Unowned>
template <typename T>
void GroupObserver<std::tuple<Owned...>, std::tuple<Unowned...>>::observeAdd()
{
	ComponentStorage* cs = m_group_ptr->getUnderlyingSparse<T>();

	/* Add ObserverBase object to SparseSet's container */
	cs->addObserverOnAdd(this); //addObserverOnAdd takes a ptr, so use this, not *this.
								//also implicit upcast to ObserverBase
}

template <typename ...Owned, typename ...Unowned>
template <typename T>
void GroupObserver<std::tuple<Owned...>, std::tuple<Unowned...>>::observeRemove()
{
	ComponentStorage* cs = m_group_ptr->getUnderlyingSparse<T>();

	/* Add ObserverBase object to SparseSet's container */
	cs->addObserverOnRemove(this); //addObserverOnAdd takes a ptr, so use this, not *this.
								   //also implicit upcast to ObserverBase
}


//NEED TO MAKE SUPPORT OWNEDSET AND UNOWNEDSET
//Private
template <typename ...Owned, typename ...Unowned>
/*virtual*/ void GroupObserver<std::tuple<Owned...>, std::tuple<Unowned...>>::notifyAdd(uint32_t entityID) //override
{
	//Each SparseSet within the Group has the entityID
	if (((std::get<SparseSet<Owned>*>(m_group_ptr->m_ownedSets))->has(entityID) && ...))
	{
		auto tmp = std::apply([](auto&&... ptrs) {return std::array<ComponentStorage*, sizeof...(ptrs)>{ptrs...}; }, m_group_ptr->m_ownedSets);

		for (size_t i{ 0 }; i < tmp.size(); ++i)
		{
			ComponentStorage* cs = tmp[i];
			
			//Already in correct spot at end of packed region, only need to increment m_len 
			if (cs->getDenseIndex(entityID) == m_group_ptr->m_len)
				continue;

			uint32_t dense_index = cs->getDenseIndex(entityID);
			uint32_t sparse_index = entityID;

			uint32_t dense_swap = m_group_ptr->m_len;
			uint32_t sparse_swap = cs->getSparseIndex(dense_swap);

			//Swap dense and data
			std::swap(cs->getDenseMutable()[dense_index], cs->getDenseMutable()[dense_swap]);
			cs->swapData(dense_index, dense_swap);
			//Swap sparse
			std::swap(cs->getSparseMutable()[sparse_index], cs->getSparseMutable()[sparse_swap]);
		}

		m_group_ptr->m_len++;
	}
}

//NEED TO MAKE SUPPORT OWNEDSET AND UNOWNEDSET
//Private
template <typename ...Owned, typename ...Unowned>
/*virtual*/ void GroupObserver<std::tuple<Owned...>, std::tuple<Unowned...>>::notifyRemove(uint32_t entityID) //override
{
	//overflow guard
	if (m_group_ptr->m_len <= 1)
	{
		m_group_ptr->m_len = 0;
		return;
	}

	//bump m_len down 1, removal of entityID. is now new boundary
	m_group_ptr->m_len--;

	//entityID is in [0,m_len] and needs to be moved outside of range
	if (std::get<0>(m_group_ptr->m_ownedSets)->getDenseIndex(entityID) <= m_group_ptr->m_len - 1)
	{
		auto tmp = std::apply([](auto&&... ptrs) {return std::array<ComponentStorage*, sizeof...(ptrs)>{ptrs...}; }, m_group_ptr->m_ownedSets);

		for (size_t i{ 0 }; i < tmp.size(); ++i)
		{
			ComponentStorage* cs = tmp[i];

			uint32_t dense_index = cs->getDenseIndex(entityID);
			uint32_t sparse_index = entityID; //getSparseIndex(dense_index)

			uint32_t dense_swap = m_group_ptr->m_len;
			uint32_t sparse_swap = cs->getSparseIndex(dense_swap);

			//Swap dense and data
			std::swap(cs->getDenseMutable()[dense_index], cs->getDenseMutable()[dense_swap]);
			cs->swapData(dense_index, dense_swap);
			//Swap sparse
			std::swap(cs->getSparseMutable()[sparse_index], cs->getSparseMutable()[sparse_swap]);
		}
	}
}

//NEED TO MAKE SUPPORT OWNEDSET AND UNOWNEDSET
template <typename ...Owned, typename ...Unowned>
void GroupObserver<std::tuple<Owned...>, std::tuple<Unowned...>>::unregisterAll()
{
	((std::get<SparseSet<Owned>*>(m_group_ptr->m_ownedSets)->remove_observerBoth(this)), ...);
}

// no op
template <typename ...Owned, typename ...Unowned>
/*virtual*/ void GroupObserver<std::tuple<Owned...>, std::tuple<Unowned...>>::clear() /*override*/ { ; }
