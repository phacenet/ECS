#pragma once

#include "Core.h"
#include "fstream"

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
	bool m_groupOwned = false;

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
		for (ObserverBase* obs : m_observersOnRemove)
			obs->notifyAdd(entityID); //changed from notifyRemove(entityID). never got added to m_matchingentities before

		uint32_t dense_index = m_sparse[entityID];
		uint32_t last_val = m_dense.back();

		/* Self - swap no-op on self in [utility.requirements] section 15.5.3.2 */
		std::swap(m_dense[dense_index], m_dense.back());
		std::swap(m_data[dense_index], m_data.back());
		uint32_t change_val = m_dense[dense_index];

		m_dense.pop_back();
		m_data.pop_back();
		
		m_sparse[change_val] = dense_index;
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

	T& get(uint32_t entityID)
	{
		uint32_t dense_index = m_sparse[entityID];
		return m_data[dense_index];
	}

	std::vector<T>& iterate() { return m_data; }

	virtual const std::vector<uint32_t>& getDense() override { return m_dense; }

	virtual std::type_index getType()
	{
		return std::type_index(typeid(T));
	}

	virtual size_t getDenseSize() { return m_dense.size(); }

	//Assumes the entityID IS in the vector
	virtual uint32_t getDenseIndex(uint32_t entityID) override { return m_sparse[entityID]; }
	virtual uint32_t getSparseIndex(uint32_t denseIndex) override { return m_dense[denseIndex]; }

	virtual void serialize(std::ofstream& outFile)
	{
		if (outFile.is_open())
		{
			// [hash]
			constexpr size_t hash = getHash<T>();
			outFile.write(reinterpret_cast<const char*>(&hash), sizeof(hash));

			// [count of entities] - count is the same for dense and data
			size_t numEntities = m_dense.size();
			outFile.write(reinterpret_cast<const char*>(&numEntities), sizeof(numEntities));

			// [entities]
			for (const uint32_t& entityID : m_dense)
				outFile.write(reinterpret_cast<const char*>(&entityID), sizeof(entityID));

			// [data]
			for (const auto& data : m_data)
				outFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
		}
	}

	virtual void deserialize(std::ifstream& inFile)
	{
		if (inFile.is_open())
		{
			size_t numEntities;
			inFile.read(reinterpret_cast<char*>(&numEntities), sizeof(numEntities));

			std::vector<uint32_t> dense;
			dense.reserve(numEntities);

			for (size_t i{ 0 }; i < numEntities; ++i)
			{
				uint32_t entityID;
				inFile.read(reinterpret_cast<char*>(&entityID), sizeof(entityID));
				dense.push_back(entityID);
			}

			std::vector<T> dense_data;
			dense_data.reserve(numEntities);

			for (size_t i{ 0 }; i < numEntities; ++i)
			{
				T data;
				inFile.read(reinterpret_cast<char*>(&data), sizeof(data));
				dense_data.emplace_back(std::move(data));
			}

			this->m_dense = dense;
			this->m_data = dense_data;
		}
	}


#ifdef _DEBUG
	const std::vector<ObserverBase*>& getObserversOnAdd() { return m_observersOnAdd; }
	const std::vector<ObserverBase*>& getObserversOnRemove() { return m_observersOnRemove; }
#endif

	//Needs privileged access to mutate dense, sparse, data
	template <typename OwnedTuple, typename UnownedTuple>
	friend class Group;

	template <typename OwnedTuple, typename UnownedTuple>
	friend class GroupObserver;

	friend class Observer;

private:
	//Helpers
	virtual std::vector<uint32_t>& getDenseMutable() override { return m_dense; }
	virtual std::vector<uint32_t>& getSparseMutable() override { return m_sparse; }
	virtual const std::vector<uint32_t>& getSparse() override { return m_sparse; }
	std::vector<T>& getMutableData() { return m_data; }

	virtual void swapData(uint32_t index1, uint32_t index2) override
	{
		std::swap(m_data[index1], m_data[index2]);
	}

	virtual void addObserverOnAdd(ObserverBase* obs) override { m_observersOnAdd.emplace_back(obs); }
	virtual void addObserverOnRemove(ObserverBase* obs) override { m_observersOnRemove.emplace_back(obs); }

	virtual void remove_observerOnAdd(ObserverBase* obs) override
	{
		auto it = std::find(m_observersOnAdd.begin(), m_observersOnAdd.end(), obs);
		assert(it != m_observersOnAdd.end() && "Supplied observerBase* that is not in m_observersOnAdd");

		if (*it != m_observersOnAdd.back())
			std::swap(*it, m_observersOnAdd.back());
		
		m_observersOnAdd.pop_back();
	}

	virtual void remove_observerOnRemove(ObserverBase* obs) override
	{
		auto it = std::find(m_observersOnRemove.begin(), m_observersOnRemove.end(), obs);
		assert(it != m_observersOnRemove.end() && "Supplied observerBase* that is not in m_observersOnAdd");

		if (*it != m_observersOnRemove.back())
			std::swap(*it, m_observersOnRemove.back());

		m_observersOnRemove.pop_back();
	}

	virtual void remove_observerBoth(ObserverBase* obs) override
	{
		auto itA = std::find(m_observersOnAdd.begin(), m_observersOnAdd.end(), obs);
		auto itR = std::find(m_observersOnRemove.begin(), m_observersOnRemove.end(), obs);

		if (itA != m_observersOnAdd.end())
		{
			if (*itA != m_observersOnAdd.back())
				std::swap(*itA, m_observersOnAdd.back());

			m_observersOnAdd.pop_back();
		}

		if (itR != m_observersOnRemove.end())
		{
			if (*itR != m_observersOnRemove.back())
				std::swap(*itR, m_observersOnRemove.back());

			m_observersOnRemove.pop_back();
		}
	}

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
	bool m_groupOwned = false;

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

	//cross compatability
	template <typename U>
	bool insert(uint32_t entityID, U&& data_value)
	{
		return this->insert(entityID);
	}

	virtual bool remove(uint32_t entityID) override
	{
		if (!has(entityID))
			return false;

		/* Before swap and pop, otherwise searches for garbage */
		for (ObserverBase* obs : m_observersOnRemove)
			obs->notifyAdd(entityID);

		uint32_t dense_index = m_sparse[entityID];
		uint32_t last_val = m_dense.back();

		/* Self - swap no-op on self in [utility.requirements] section 15.5.3.2 */
		std::swap(m_dense[dense_index], m_dense.back());
		uint32_t change_val = m_dense[dense_index];

		m_dense.pop_back();
		
		m_sparse[change_val] = dense_index;
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

	virtual const std::vector<uint32_t>& getDense() override { return m_dense; }

	virtual std::type_index getType()
	{
		return std::type_index(typeid(T));
	}

	virtual size_t getDataSize() { return 0; }

	virtual size_t getDenseSize() { return m_dense.size(); }


	virtual void serialize(std::ofstream& outFile)
	{
		if (outFile.is_open())
		{
			// [hash]
			constexpr size_t hash = getHash<T>();
			outFile.write(reinterpret_cast<const char*>(&hash), sizeof(hash));

			// [count of entities] - count is the same for dense and data
			uint32_t numEntities = m_dense.size();
			outFile.write(reinterpret_cast<const char*>(&numEntities), sizeof(numEntities));

			// [entities]
			for (const uint32_t& entityID : m_dense)
				outFile.write(reinterpret_cast<const char*>(&entityID), sizeof(entityID));
		}
	}


	virtual void deserialize(std::ifstream& inFile)
	{
		if (inFile.is_open())
		{
			uint32_t numEntities;
			inFile.read(reinterpret_cast<char*>(&numEntities), sizeof(numEntities));

			std::vector<uint32_t> dense;
			dense.reserve(numEntities);

			for (size_t i{ 0 }; i < numEntities; ++i)
			{
				uint32_t entityID;
				inFile.read(reinterpret_cast<char*>(&entityID), sizeof(entityID));
				dense.push_back(entityID);
			}

			this->m_dense = dense;
		}
	}



#ifdef _DEBUG
	const std::vector<ObserverBase*>& getObserversOnAdd() { return m_observersOnAdd; }
	const std::vector<ObserverBase*>& getObserversOnRemove() { return m_observersOnRemove; }
#endif

	//Needs privileged access to mutate dense, sparse
	template <typename OwnedTuple, typename UnownedTuple>
	friend class Group;

	template <typename OwnedTuple, typename UnownedTuple>
	friend class GroupObserver;

	friend class Observer;

private:
	
	//Helpers
	virtual std::vector<uint32_t>& getDenseMutable() override { return m_dense; }
	virtual std::vector<uint32_t>& getSparseMutable() override { return m_sparse; }
	virtual const std::vector<uint32_t>& getSparse() override { return m_sparse; }

	virtual void swapData(uint32_t index1, uint32_t index2) override { ; } //no op

	virtual uint32_t getDenseIndex(uint32_t entityID) override { return m_sparse[entityID]; }
	virtual uint32_t getSparseIndex(uint32_t denseIndex) override { return m_dense[denseIndex]; }

	virtual void addObserverOnAdd(ObserverBase* obs) override { m_observersOnAdd.emplace_back(obs); }
	virtual void addObserverOnRemove(ObserverBase* obs) override { m_observersOnRemove.emplace_back(obs); }

	virtual void remove_observerOnAdd(ObserverBase* obs) override
	{
		auto it = std::find(m_observersOnAdd.begin(), m_observersOnAdd.end(), obs);
		assert(it != m_observersOnAdd.end() && "Supplied observerBase* that is not in m_observersOnAdd");

		if (*it != m_observersOnAdd.back())
			std::swap(*it, m_observersOnAdd.back());

		m_observersOnAdd.pop_back();
	}

	virtual void remove_observerOnRemove(ObserverBase* obs) override
	{
		auto it = std::find(m_observersOnRemove.begin(), m_observersOnRemove.end(), obs);
		assert(it != m_observersOnRemove.end() && "Supplied observerBase* that is not in m_observersOnAdd");

		if (*it != m_observersOnRemove.back())
			std::swap(*it, m_observersOnRemove.back());

		m_observersOnRemove.pop_back();
	}

	//Only one that performs safety checks for std::find
	virtual void remove_observerBoth(ObserverBase* obs) override
	{
		auto itA = std::find(m_observersOnAdd.begin(), m_observersOnAdd.end(), obs);
		auto itR = std::find(m_observersOnRemove.begin(), m_observersOnRemove.end(), obs);

		if (itA != m_observersOnAdd.end())
		{
			if (*itA != m_observersOnAdd.back())
				std::swap(*itA, m_observersOnAdd.back());

			m_observersOnAdd.pop_back();
		}

		if (itR != m_observersOnRemove.end())
		{
			if (*itR != m_observersOnRemove.back())
				std::swap(*itR, m_observersOnRemove.back());

			m_observersOnRemove.pop_back();
		}
	}
};