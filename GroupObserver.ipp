

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

//Ctor
template <typename ...Args>
GroupObserver<Args...>::GroupObserver(Group<Args...>& group)
	: m_group_ptr(&group) {}


template <typename ...Args>
template <typename T>
void GroupObserver<Args...>::observeAdd()
{
	ComponentStorage* cs = m_group_ptr->getUnderlyingSparse<T>();
	SparseSet<T>* ss = static_cast<SparseSet<T>*>(cs);

	/* Add ObserverBase object to SparseSet's container */
	ss->addObserverOnAdd(this); //addObserverOnAdd takes a ptr, so use this, not *this.
								//also implicit upcast to ObserverBase
}

template <typename ...Args>
template <typename T>
void GroupObserver<Args...>::observeRemove()
{
	ComponentStorage* cs = m_group_ptr->getUnderlyingSparse<T>();
	SparseSet<T>* ss = static_cast<SparseSet<T>*>(cs);

	/* Add ObserverBase object to SparseSet's container */
	ss->addObserverOnRemove(this); //addObserverOnAdd takes a ptr, so use this, not *this.
								   //also implicit upcast to ObserverBase
}

template <typename ...Args>
/*virtual*/ void GroupObserver<Args...>::notifyAdd(uint32_t entityID) //override
{

}

template <typename ...Args>
/*virtual*/ void GroupObserver<Args...>::notifyRemove(uint32_t entityID) //override
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
			uint32_t sparse_index = cs->getSparseIndex(dense_index);

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

template <typename ...Args>
template <typename T>
void GroupObserver<Args...>::each(T&& lambda)
{

}

// no op
template <typename ...Args>
/*virtual*/ void GroupObserver<Args...>::clear() /*override*/ { ; }
