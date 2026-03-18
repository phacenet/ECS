

/*
class Observer : public ObserverBase
{
private:
	World* m_world_ptr = nullptr;
	std::vector<uint32_t> m_matchingEntities;

	public:
*/

/* Ctor */
Observer::Observer(World& world)
	: m_world_ptr(&world) {
}

/* Add "add" observer to corresponding SparseSet<T>'s m_observersOnAdd */
template <typename T>
void Observer::observeAdd()
{
	ComponentStorage* cs = m_world_ptr->getStorage<T>();
	SparseSet<T>* ss = static_cast<SparseSet<T>*>(cs);

	/* Add ObserverBase object to SparseSet's container */
	ss->addObserverOnAdd(this); //addObserverOnAdd takes a ptr, so use this, not *this.
	//also implicit upcast to ObserverBase
}

/* Add "remove" observer to corresponding SparseSet<T>'s m_observersOnRemove */
template <typename T>
void Observer::observeRemove()
{
	ComponentStorage* cs = m_world_ptr->getStorage<T>();
	SparseSet<T>* ss = static_cast<SparseSet<T>*>(cs);

	/* Add ObserverBase object to SparseSet's container */
	ss->addObserverOnRemove(this);  //addObserverOnAdd takes a ptr, so use this, not *this.
	//also implicit upcast to ObserverBase
}

/* Method to push entityID into m_matchingEntities */
void Observer::notifyAdd(uint32_t entityID) { m_matchingEntities.push_back(entityID); }

/* Method to remove entityID into m_matchingEntities */
void Observer::notifyRemove(uint32_t entityID)
{
	auto it = std::find(m_matchingEntities.begin(), m_matchingEntities.end(), entityID);
	if (it != m_matchingEntities.end())
		m_matchingEntities.erase(it);
}

/* Clears all values from observer's m_matchingEntities container */
void Observer::clear()
{
	m_matchingEntities.clear();
}

/* For running lambda/std::function on each entityID in m_matchingEntities */
template<typename T>
void Observer::each(T&& func)
{
	for (uint32_t i{ 0 }; i < m_matchingEntities.size(); ++i)
	{
		uint32_t entityID = m_matchingEntities[i];
		std::forward<T>(func)(entityID);
	}
}
