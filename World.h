#pragma once

#include "Core.h"
#include "ComponentStorage.h"
#include "SparseSet.h"
#include "ComponentIndexing.h"
#include "get_t.h"


/* Forward Declarations */
namespace DebugFunctions
{
	struct Access;
}

template <typename OwnedTuple, typename UnownedTuple>
class Group;

template <typename ...Args>
class View;

class World
{
private:
	std::vector<ComponentStorage*> m_components;

	uint32_t m_nextEntityID = 0;
	std::unordered_set<uint32_t> m_aliveIDs;
	std::stack<uint32_t, std::vector<uint32_t>> m_freeIDs; //container adaptor, wraps container and exposes: empty, size, push, pop, top. //<stored type, which container is wrapped>
	
	/* Helpers */
	/* ============================================================ */
	bool _is_alive_(uint32_t entityID) { return m_aliveIDs.contains(entityID); }
	
	template <typename T>
	bool _registered_component_()
	{ 
		uint32_t index = getTypeIndex<T>();
		/* Within range and not nullptr means already registered */
		if (m_components.empty())
			return false;

		return (index <= (m_components.size() - 1) && m_components[index] != nullptr);
	}

	template <typename T>
	void _lazy_register_()
	{
		if (!_registered_component_<T>())
			registerComponent<T>();
	}

	void _register_aliveID_();

	void _register_freeID_();
	/* ============================================================ */
public:
	uint32_t createEntity();

	std::vector<uint32_t> createEntities(uint32_t numEntities);

	bool destroyEntity(uint32_t entityID);

	bool destroyEntities(std::initializer_list<uint32_t> entityIDs);

	template <typename T>
	bool registerComponent();

	template <typename T>
	bool addComponent(uint32_t entityID, T&& data);
	
	/* Overload for default constructed T */
	template <typename T>
	bool addComponent(uint32_t entityID);

	/* Overload for adding tags without data */
	template <typename T>
	requires std::is_base_of_v<TagBase, T>
	bool addComponent(uint32_t entityID);

	/* Add/register one or more Component(s) to one or more entityID(s). Data values are the same for all entityIDs. */
	template <typename ...Args>
	bool addComponents(std::initializer_list<uint32_t> entityIDs, Args&&... args);

	/* Default constructed Args... */
	template <typename ...Args>
	void addComponents(std::initializer_list<uint32_t> entityIDs);

	template <typename T>
	bool removeComponent(uint32_t entityID);

	template <typename ...Args>
	bool removeComponents(std::initializer_list<uint32_t> entityIDs);

	/* User is responsible for passing valid entityID and type */
	template <typename T>
	T& getComponentData(uint32_t entityID);

	template <typename T>
	ComponentStorage* getStorage();

	template <typename ...Args>
	View<Args...> view();

	template <typename ...Owned>
	Group<std::tuple<Owned...>, std::tuple<>> group();

	template <typename ...Owned, typename ...Unowned>
	Group<std::tuple<Owned...>, std::tuple<Unowned...>> group(get_t<Unowned...>);

	ComponentStorage* getComponent(size_t pos);

	template <typename T>
	void visit(uint32_t entityID, T&& callback);

	template <typename T>
	void visit(T&& callback);

	template <typename... Args>
	void serialize(const char* file);

	template <typename... Args>
	void deserialize(const char* file);


	#ifdef _DEBUG
	bool isAlive(uint32_t entityID);
	#endif

	/* Ctor */
	World() = default;

	/* Dtor */
	~World()
	{
		/* Clean up Heap allocated Components* */
		for (auto& comp : m_components)
			delete comp;

	}

	World(const World& other) = delete;
	World operator=(const World& other) = delete;
	World(World&& other) = delete; //disabled for now
	World& operator=(World&& other) = delete; //disabled for now
	
	/* Prevent incorrect user call */
	std::vector<uint32_t> getEntitiesWith() = delete;

	/* Gives DebugFunctions:Access access to private members without polluting class */
	friend struct DebugFunctions::Access;

	friend class Serialize;

};

#include "World.ipp"
#include "View.h"
#include "Serialize.h"