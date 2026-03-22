

/*
class Observer : public ObserverBase
{
private:
	World* m_world_ptr = nullptr;
	std::vector<uint32_t> m_matchingEntities;
	std::vector<ComponentStorage*> m_monitoredComponents;

	public:
*/

/* Ctor */
Observer::Observer(World& world)
	: m_world_ptr(&world) {}

/* Add "add" observer to corresponding SparseSet<T>'s m_observersOnAdd */
template <typename T>
void Observer::observeAdd()
{
	ComponentStorage* cs = m_world_ptr->getStorage<T>();

	/* Add ObserverBase object to SparseSet's container */
	cs->addObserverOnAdd(this); //addObserverOnAdd takes a ptr, so use this, not *this.
								//also implicit upcast to ObserverBase

	checkIfMonitored(cs);
}

/* Add "remove" observer to corresponding SparseSet<T>'s m_observersOnRemove */
template <typename T>
void Observer::observeRemove()
{
	ComponentStorage* cs = m_world_ptr->getStorage<T>();

	/* Add ObserverBase object to SparseSet's container */
	cs->addObserverOnRemove(this);  //addObserverOnAdd takes a ptr, so use this, not *this.
									//also implicit upcast to ObserverBase

	checkIfMonitored(cs);
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

size_t Observer::size()
{
	return m_matchingEntities.size();
}

template <typename T>
void Observer::unregister(ObserverType type) //no default arg in implementation allowed, only in declaration
{
	ComponentStorage* cs = m_world_ptr->getStorage<T>();

	if (type == ObserverType::ONADD)
		cs->remove_observerOnAdd(this);

	else if (type == ObserverType::ONREMOVE)
		cs->remove_observerOnRemove(this);

	else if (ObserverType::BOTH)
	{
		//bypasses safety check performed in remove_observerBoth
		cs->remove_observerOnAdd(this);
		cs->remove_observerOnRemove(this);
	}

	else //SAFE
		cs->remove_observerBoth(this);

}

//SAFE
void Observer::unregisterAll()
{
	for (auto& cs : m_monitoredComponents)
		cs->remove_observerBoth(this);
}

//Private
//Helper for repeated check
void Observer::checkIfMonitored(ComponentStorage* cs)
{
	if (std::find(m_monitoredComponents.begin(), m_monitoredComponents.end(), cs) == m_monitoredComponents.end());
		m_monitoredComponents.push_back(cs);
}