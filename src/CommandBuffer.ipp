

/*
class CommandBuffer
{
private:
	std::vector<std::function<void(World& world)>> m_funcs;

public:
*/

CommandBuffer::CommandBuffer(World& world)
{
	m_owner_world = &world;
}

void CommandBuffer::destroy(uint32_t entityID)
{
	auto callback = [entityID, this]()
		{
			m_owner_world->destroyEntity(entityID);
		};
	m_funcs.emplace_back(callback);
}

template <typename T>
void CommandBuffer::remove(uint32_t entityID)
{
	auto callback = [entityID, this]()
		{
			m_owner_world->removeComponent<T>(entityID);
		};

	m_funcs.emplace_back(callback);
}

template <typename ...Args>
void CommandBuffer::emplace(uint32_t entityID)
{
	auto callback = [entityID, this]()
		{
			m_owner_world->addComponents<Args...>({ entityID });
		};
	m_funcs.emplace_back(callback);
}

void CommandBuffer::flush()
{
	for (auto& func : m_funcs)
	{
		func();
	}
	m_funcs.clear();
}
