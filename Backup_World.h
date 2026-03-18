#pragma once

#include "Core.h"
#include "ComponentStorage.h"
#include "SparseSet.h"
#include "ComponentIndexing.h"
#include "View.h"

/* Declaration */
namespace DebugFunctions
{
	struct Access;
}

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
		return (index <= (m_components.size() - 1) && m_components[index] != nullptr);
	}

	template <typename T>
	void _lazy_register_()
	{
		if (!_registered_component_<T>())
			registerComponent<T>();
	}
	/* ============================================================ */
public:
	/* Create new entity */
	uint32_t createEntity()
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

	/* Create multiple entities at once and return IDs in a vector */
	std::vector<uint32_t> createEntities(uint32_t numEntities)
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

	/* Destroy an entityID and remove all of its Components */
	bool destroyEntity(uint32_t entityID)
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

	/* Destroy multiple entityIDs at once */
	bool destroyEntities(std::initializer_list<uint32_t> entityIDs)
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
	bool registerComponent()
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

	/* Register component AND add that component to the entity if not already registered,
		otherwise just add */
	template <typename T>
	bool addComponent(uint32_t entityID, T&& data)
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
	bool addComponent(uint32_t entityID)
	{
		return addComponent<T>(entityID, T{});
	}

	/* Overload for adding tags without data */
	template <typename T>
		requires std::is_base_of_v<TagBase, T>
	bool addComponent(uint32_t entityID)
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
	bool addComponents(std::initializer_list<uint32_t> entityIDs, Args&&... args)
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

	/* Removes Component from entity */
	template <typename T>
	bool removeComponent(uint32_t entityID)
	{
		if (!_is_alive_(entityID) || !_registered_component_<T>())
			return false;

		uint32_t index = getTypeIndex<T>();
		ComponentStorage* cs = m_components[index];
		return (cs->remove(entityID)); //remove returns true if found entityID and removed
	}

	/* Remove one or more Component(s) from one or more entityID(s)  */
	template <typename ...Args>
	bool removeComponents(std::initializer_list<uint32_t> entityIDs)
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

	/* Return T& to Component data value for that entityID, user is responsible for passing valid entityID and type */
	template <typename T>
	T& getComponentData(uint32_t entityID)
	{
		ComponentStorage* cs = m_components.at(getTypeIndex<T>());
		SparseSet<T>* ss = static_cast<SparseSet<T>*>(cs);
		return ss->get(entityID); //changed to return ref
	}

	/* Modifies user-vector to contain entityID of all entities with specified Components */
	template <typename... Args>
	void getEntitiesWith(std::vector<uint32_t>& entities)
	{
		entities.clear();

		/* Empty vector return on bad component pass */
		if (!(_registered_component_<Args>() && ...))
			return;

		std::array<size_t, sizeof...(Args)> sizes;
		sizes = { (m_components.at(getTypeIndex<Args>())->getDataSize())... };

		std::array<ComponentStorage*, sizeof...(Args)> bffr;
		bffr = { (m_components.at(getTypeIndex<Args>()))... };

		/* Find ComponentStorage with smallest size for starting point of intersection algorithm */
		size_t min_size = UINT32_MAX;
		size_t min_size_index = 0;
		for (size_t i{ 0 }; i < bffr.size(); ++i)
		{
			if (min_size > sizes[i])
			{
				min_size = sizes[i];
				min_size_index = i;
			}
		}

		if (min_size == 0)
			return; //empty, no unions

		ComponentStorage* smallest_dense = bffr[min_size_index];
		entities.reserve(min_size);
		entities = smallest_dense->getDense();

		/* Compares result with provided comparison vector, modifies result to only contain entityIDs found in BOTH vectors */
		auto intersect_with = [&](const std::vector<uint32_t>& comparison)
			{
				std::unordered_set<uint32_t> comp{ comparison.begin(), comparison.end() };

				for (auto it = entities.begin(); it != entities.end(); /* Empty intentionally */)
				{
					if (!comp.contains(*it))
						it = entities.erase(it);
					else
						++it;
				}
			};

		/* Iterate intersect_with over all ComponentStorage in bffr, except self (smallest_dense) */
		for (size_t i{ 0 }; i < bffr.size(); ++i)
		{
			/* Guard self comparison */
			if (i == min_size_index)
				continue;
			else
				intersect_with(bffr[i]->getDense());
		}
	}

	/* Modify Component with user-created lambda */
	template <typename ...Args, typename T>
	void forEach(T&& lambda)
	{
		std::vector<uint32_t> result;
		getEntitiesWith<Args...>(result); //result is modified and has all the matching entityIDs

		for (uint32_t i{ 0 }; i < result.size(); ++i)
		{
			uint32_t entityID = result[i];
			lambda(entityID, getComponentData<Args>(entityID)...);
		}
	}

	template <typename T>
	ComponentStorage* getStorage()
	{
		_lazy_register_<T>();

		ComponentStorage* cs = m_components.at(getTypeIndex<T>());
		return cs;
	}

	template <typename ...Args>
	View<Args...> view()
	{
		return View<Args...>();
	}


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

};

#include "World.ipp"
