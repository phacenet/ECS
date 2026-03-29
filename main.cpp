
#include "Core.h"

#include "World.h"
#include "DebugFunctions.h"
#include "Observer.h"
#include "Sink.h"
#include "Signal.h"
#include "Handler.h"
#include "Dispatcher.h"
#include "Group.h"
#include "GroupObserver.h"
#include "ExcludedGroupView.h"
#include "CommandBuffer.h"


struct Health
{
	float hlth = 100.0f;
};

struct Stamina
{
	float stmna = 100.0f;
};

struct PlayerTag : public TagBase {};

struct DamageEvent
{
	uint32_t entityID;
	float amount;
};

struct HealthSystem
{
	void onDamage(const DamageEvent& e)
	{
		std::cout << "Entity" << e.entityID << " took " << e.amount << " damage\n";
	}
};

void freeOnDamage(const DamageEvent& e)
{
	std::cout << "Entity" << e.entityID << " took " << e.amount << " damage\n";
}
void freeAddHealth(const DamageEvent& e)
{
	std::cout << "Entity" << e.entityID << " gained " << e.amount << " health\n";
}

int main()
{
	World world;

	world.createEntities(5); //0-4
	world.registerComponent<Health>();
	world.registerComponent<PlayerTag>();


	world.addComponents<Health, Stamina, PlayerTag>({ 0, 1, 2});
	DebugFunctions::Access::view_all_IDs(world);


	world.visit([](uint32_t entityID, std::type_index type)
		{
			std::cout << "entityID " << entityID << " has type " << type.name() << "\n";
		});
	world.serialize<Health, Stamina, PlayerTag>();
	std::cout << "Serialized\n";

	World world2;
	world2.deserialize<Stamina, PlayerTag, Health>();
	DebugFunctions::Access::view_all_IDs(world2);

	world.visit([](uint32_t entityID, std::type_index type)
		{
			std::cout << "entityID " << entityID << " has type " << type.name() << "\n";
		});
}
