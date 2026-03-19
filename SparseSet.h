#pragma once

#include "Core.h"
#include "ComponentStorage.h"
#include "TagBase.h"
#include "ObserverBase.h"

template <typename T>
class SparseSet : public ComponentStorage
{
private:
	std::vector<uint32_t> m_sparse;
	std::vector<uint32_t> m_dense;
	std::vector<T> m_data;
	std::vector<ObserverBase*> m_observersOnAdd;
	std::vector<ObserverBase*> m_observersOnRemove;

public:
	
	template <typename U>
	bool insert(uint32_t entityID, U&& data_value)
	{
		//ensure U is same type as T, just rvalue vs lvalue
		static_assert(std::is_same_v<T, std::remove_cvref_t<U>>);

		/* Duplicate entry */
		if (has(entityID))
			return false;

		if (entityID >= m_sparse.size())
			m_sparse.resize(entityID + 1, UINT32_MAX);

		m_sparse[entityID] = static_cast<uint32_t>(m_dense.size());
		m_dense.emplace_back(entityID);
		m_data.emplace_back(std::forward<U>(data_value));

		/* Add entityID to each observer's matchingEntities list */
		for (ObserverBase* obs : m_observersOnAdd)
			obs->notifyAdd(entityID);

		return true;
	}

	virtual bool remove(uint32_t entityID) override
	{
		if (!has(entityID))
			return false;

		/* Before swap and pop, otherwise searches for garbage */
		for (ObserverBase* obs : m_observersOnAdd)
			obs->notifyRemove(entityID);

		uint32_t dense_index = m_sparse[entityID];
		uint32_t last_val = m_dense[m_dense.size() - 1];

		/* Self - swap no-op on self in [utility.requirements] section 15.5.3.2 */
		std::swap(m_dense[dense_index], m_dense[m_dense.size() - 1]);
		std::swap(m_data[dense_index], m_data[m_data.size() - 1]);
		uint32_t change_val = m_dense[dense_index];

		m_dense.pop_back();
		m_data.pop_back();
		
		m_sparse[last_val] = dense_index;
		m_sparse[entityID] = UINT32_MAX;

		return true;
	}

	virtual bool has(uint32_t entityID) override
	{
		/* Bounds */
		if (entityID >= m_sparse.size())
			return false;

		/* O(1) */
		if (m_sparse[entityID] != UINT32_MAX)
			return true;

		return false;
	}

	/* Returned T* before changing to T& */
	T& get(uint32_t entityID)
	{
		uint32_t dense_index = m_sparse[entityID];
		return m_data[dense_index];
	}

	std::vector<T>& iterate() { return m_data; }

	//virtual size_t getDataSize() override { return m_data.size(); }

	const std::vector<uint32_t>& getDense() { return m_dense; }

	void addObserverOnAdd(ObserverBase* obs) { m_observersOnAdd.emplace_back(obs); }
	void addObserverOnRemove(ObserverBase* obs) { m_observersOnRemove.emplace_back(obs); }

	virtual std::type_index getType()
	{
		return std::type_index(typeid(T));
	}

	virtual size_t getDenseSize() { return m_dense.size(); }

};


/* Partial specialization for TagBase, doesn't need data */
template <typename T>
requires std::is_base_of_v<TagBase, T>
class SparseSet<T> : public ComponentStorage
{
private:
	std::vector<uint32_t> m_sparse;
	std::vector<uint32_t> m_dense;
	std::vector<ObserverBase*> m_observersOnAdd;
	std::vector<ObserverBase*> m_observersOnRemove;

public:

	bool insert(uint32_t entityID)
	{
		/* Duplicate entry */
		if (has(entityID))
			return false;

		if (entityID >= m_sparse.size())
			m_sparse.resize(entityID + 1, UINT32_MAX);

		m_sparse[entityID] = static_cast<uint32_t>(m_dense.size());
		m_dense.emplace_back(entityID);

		/* Add entityID to each observer's matchingEntities list */
		for (ObserverBase* obs : m_observersOnAdd)
			obs->notifyAdd(entityID);

		return true;
	}

	virtual bool remove(uint32_t entityID) override
	{
		if (!has(entityID))
			return false;

		/* Before swap and pop, otherwise searches for garbage */
		for (ObserverBase* obs : m_observersOnAdd)
			obs->notifyRemove(entityID);

		uint32_t dense_index = m_sparse[entityID];
		uint32_t last_val = m_dense[m_dense.size() - 1];

		/* Self - swap no-op on self in [utility.requirements] section 15.5.3.2 */
		std::swap(m_dense[dense_index], m_dense[m_dense.size() - 1]);
		uint32_t change_val = m_dense[dense_index];

		m_dense.pop_back();

		m_sparse[last_val] = dense_index;
		m_sparse[entityID] = UINT32_MAX;

		return true;
	}

	virtual bool has(uint32_t entityID) override
	{
		/* Bounds */
		if (entityID >= m_sparse.size())
			return false;

		/* O(1) */
		if (m_sparse[entityID] != UINT32_MAX)
			return true;

		return false;
	}

	const std::vector<uint32_t>& getDense() { return m_dense; }

	void addObserverOnAdd(ObserverBase* obs) { m_observersOnAdd.emplace_back(obs); }
	void addObserverOnRemove(ObserverBase* obs) { m_observersOnRemove.emplace_back(obs); }

	virtual std::type_index getType()
	{
		return std::type_index(typeid(T));
	}

	virtual size_t getDataSize() { return 0; }

	virtual size_t getDenseSize() { return m_dense.size(); }

};