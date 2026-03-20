#pragma once

#include "Core.h"
#include "World.h"
#include "Dispatcher.h"
#include "Signal.h"
#include "Sink.h"

namespace DebugFunctions
{
	/* Struct has private access to World  - all members should be static so the struct never has to be instantiated */
	struct Access
	{
		/* Prints all alive and freed IDs to the console. Expensive: destroying and rebuilding stack and an additional vector */
		static void view_all_IDs(World& world)
		{
			/* Alive */
			auto seperator = "";
			std::cout << "Alive IDs: [";

			for (const auto& e : world.m_aliveIDs)
			{
				std::cout << seperator << e;
				seperator = ", ";
			}
			std::cout << "]\n";

			/* Free/Recycled */
			seperator = "";
			std::cout << "Free IDs: [";

			/* Going to pop a value off of free_IDs and push_back into tmp, then rebuild stack in reverse order of tmp so it remains "unchanged" */
			std::vector<uint32_t> tmp;
			tmp.reserve(world.m_freeIDs.size());

			/* Destroy stack and build vector */
			while (!world.m_freeIDs.empty())
			{
				uint32_t top_val = world.m_freeIDs.top();
				tmp.push_back(top_val);
				world.m_freeIDs.pop();

				std::cout << seperator << top_val;
				seperator = ", ";
			}
			std::cout << "]\n";

			/* Rebuild stack */
			while (!tmp.empty())
			{
				uint32_t top_val = tmp.back();
				world.m_freeIDs.emplace(top_val);
				tmp.pop_back();
			}
		}

		static void view_next_freeID(World& world)
		{
			std::cout << "ID on next createEntity(): " << world.m_nextEntityID << "\n";
		}

		static void view_all_components(World& world)
		{
			const char* seperator = "";
			std::cout << "Component types: [";

			for (auto& comp : world.m_components)
			{
				std::cout << seperator << comp->getType().name();
				if (std::strlen(seperator) == 0)
				{
					seperator = ", ";
				}
			}
			std::cout << "]\n";
		}

		template <typename T>
		static void view_dense_size(World& world)
		{
			uint32_t index = getTypeIndex<T>();
			std::cout << "Component: \"" << std::type_index(typeid(T)).name() << "\" has a dense size of: " << world.m_components[index]->getDenseSize() << "\n";
		}
		
	};
}

