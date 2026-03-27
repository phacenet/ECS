
uint32_t World::createEntity()
{
	uint32_t newID;

	/* Recycle ID */
	if (!m_freeIDs.empty())
	{
		newID = m_freeIDs.top();
		m_freeIDs.pop();
	}
	/* Use next free entityID */
	else
	{
		newID = m_nextEntityID;
		++m_nextEntityID;
	}
	/* Insert newID into aliveIDs for both cases */
	m_aliveIDs.insert(newID);
	return newID;
}

std::vector<uint32_t> World::createEntities(uint32_t numEntities)
{
	std::vector<uint32_t> new_entities;
	new_entities.reserve(numEntities); //prevent potential reallocs

	if (numEntities == 0)
		return new_entities;

	for (uint32_t i{ 0 }; i < numEntities; ++i)
	{
		new_entities.push_back(createEntity());
	}

	return new_entities;
}

bool World::destroyEntity(uint32_t entityID)
{
	bool alive = m_aliveIDs.contains(entityID);
	if (!alive)
		return false;

	/* Remove from ComponentStorage (underlying SparseSets) first */
	for (auto& comp : m_components)
	{
		comp->remove(entityID); //performs underlying has(entityID) check 
	}

	/* Add to freeIDs and extract from aliveIDs */
	m_freeIDs.push(entityID);
	m_aliveIDs.extract(entityID);
	return true;
}

bool World::destroyEntities(std::initializer_list<uint32_t> entityIDs)
{
	bool success = true;
	for (auto it = entityIDs.begin(); it != entityIDs.end(); ++it)
	{
		bool destroyed = destroyEntity(*it);
		if (!destroyed)
			success = false;
	}
	return success;
}

/* Register a Component for later use, without assigning to an entityID */
template <typename T>
bool World::registerComponent()
{
	uint32_t index = getTypeIndex<T>(); //registers index number

	/* Already registered */
	if (index < m_components.size() && m_components[index] != nullptr)
		return false;

	ComponentStorage* component = new SparseSet<T>; //already returns ptr from new, heap alloc so clean up in dtor

	/* Index within alloc'd size, no resize */
	if (index < m_components.size())
	{
		m_components[index] = component; //ptr, no std::move
		return true;
	}

	/* Needs resize */
	m_components.resize(index + 1, nullptr);
	m_components[index] = component;
	return true;
}

template <typename T>
bool World::addComponent(uint32_t entityID, T&& data)
{
	/* Invalid entityID */
	if (!_is_alive_(entityID))
		return false;

	/* No op if already registered */
	registerComponent<T>();

	uint32_t index = getTypeIndex<T>();

	/* Cast ComponentStorage to underlying SparseSet and insert */
	ComponentStorage* cs = m_components[index];
	SparseSet<T>* ss = static_cast<SparseSet<T>*>(cs);

	return(ss->insert(entityID, std::forward<T>(data))); //insert returns true on successful insert	
}

/* Overload for default constructed T */
template <typename T>
bool World::addComponent(uint32_t entityID)
{
	return addComponent<T>(entityID, T{});
}

/* Overload for adding tags without data */
template <typename T>
	requires std::is_base_of_v<TagBase, T>
bool World::addComponent(uint32_t entityID)
{
	/* Invalid entityID */
	if (!_is_alive_(entityID))
		return false;

	registerComponent<T>();

	/* Cast ComponentStorage to underlying SparseSet and insert */
	ComponentStorage* cs = m_components.at(std::type_index(typeid(T)));
	SparseSet<T>* ss = static_cast<SparseSet<T>*>(cs);

	bool return_state = ss->insert(entityID); //insert returns true on successful insert
	return return_state;
}

/* Add/register one or more Component(s) to one or more entityID(s). Data values are the same for all entityIDs. */
template <typename ...Args>
bool World::addComponents(std::initializer_list<uint32_t> entityIDs, Args&&... args)
{
	bool success = true;
	for (auto it = entityIDs.begin(); it != entityIDs.end(); ++it)
	{
		bool added = (addComponent(*it, std::remove_cvref_t<Args>(args)) && ...); //copy instead of move since multiple iterations on this data
		if (!added)
			success = false;
	}
	return success;
}

/* Default constructed Args... */
template <typename ...Args>
void World::addComponents(std::initializer_list<uint32_t> entityIDs)
{
	for (auto it = entityIDs.begin(); it != entityIDs.end(); ++it)
		((addComponent<Args>(*it, Args{})), ...);
}

template <typename T>
bool World::removeComponent(uint32_t entityID)
{
	if (!_is_alive_(entityID) || !_registered_component_<T>())
		return false;

	uint32_t index = getTypeIndex<T>();
	ComponentStorage* cs = m_components[index];
	return (cs->remove(entityID)); //remove returns true if found entityID and removed
}

template <typename ...Args>
bool World::removeComponents(std::initializer_list<uint32_t> entityIDs)
{
	bool success = true;
	for (auto it = entityIDs.begin(); it != entityIDs.end(); ++it)
	{
		bool removed = (removeComponent<Args>(*it) && ...);
		if (!removed)
			success = false;
	}
	return success;
}

/* User is responsible for passing valid entityID and type */
template <typename T>
T& World::getComponentData(uint32_t entityID)
{
	ComponentStorage* cs = m_components.at(getTypeIndex<T>());
	SparseSet<T>* ss = static_cast<SparseSet<T>*>(cs);
	return ss->get(entityID); //changed to return ref
}

template <typename T>
ComponentStorage* World::getStorage()
{
	_lazy_register_<T>();

	ComponentStorage* cs = m_components.at(getTypeIndex<T>());
	return cs;
}

template <typename ...Args>
View<Args...> World::view()
{
	return View<Args...>(*this);
}

template <typename ...Owned>
Group<std::tuple<Owned...>, std::tuple<>> World::group()
{
	return Group<std::tuple<Owned...>, std::tuple<>>(*this, get<>);
}

template <typename ...Owned, typename ...Unowned>
Group<std::tuple<Owned...>, std::tuple<Unowned...>> World::group(get_t<Unowned...>)
{
	return Group<std::tuple<Owned...>, std::tuple<Unowned...>>(*this, get<Unowned...>);
}

ComponentStorage* World::getComponent(size_t pos)
{
	return m_components.at(pos);
}

template <typename T>
void World::visit(uint32_t entityID, T&& callback)
{
	for (ComponentStorage* cs : m_components)
	{
		if (cs->has(entityID))
		{
			if constexpr (std::is_invocable_v<T, std::type_index>)
			{
				std::type_index type = cs->getType();
				callback(type);
			}

			else if constexpr (std::is_invocable_v < T, uint32_t>)
				callback(entityID);

			else
				static_assert(always_false<T>, "Unsupported callable signature");
		}
	}
}

template <typename T>
void World::visit(T&& callback)
{
	for (ComponentStorage* cs : m_components)
	{
		if constexpr (std::is_invocable_v<T, uint32_t, std::type_index>)
		{
			std::type_index type = cs->getType();
			std::vector<uint32_t> dense_vec = cs->getDense();

			for (uint32_t entityID : dense_vec)
				callback(entityID, type);
		}

		else
			static_assert(always_false<T>, "Unsupported callable signature");
	}
}

template <typename... Args>
void World::serialize(const char* file)
{
	Serialize s;
	s.serialize<Args...>(this, file);
}


template <typename... Args>
void World::deserialize(const char* file)
{
	Serialize s;
	s.deserialize<Args...>(*this, file);
}

#ifdef _DEBUG
bool World::isAlive(uint32_t entityID)
{
	return m_aliveIDs.contains(entityID);
}
#endif