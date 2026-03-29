
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


	world.addComponents<Health, PlayerTag>({ 0, 1, 2});
	DebugFunctions::Access::view_all_IDs(world);

	/* Group releases ownership when it is destroyed */
	{
		auto group = world.group<Health, PlayerTag>(); //auto group = Group<Position, Velocity>(world); OR Group<Health, Stamina> group(world);]
	}

	auto group = world.group<Health, PlayerTag>(get<Health>);

	group.each([](Health& h)
		{
			h.hlth += 5.55f;
		});

	group.each([](uint32_t entityID, Health& h)
		{
			h.hlth += 1.23f;
			std::cout << "entityID: " << entityID << ", h.hlth: " << h.hlth << "\n";
		});

	bool groupHas = group.contains(0);
	auto& groupHealth = group.get<Health>(0); //need disallow playertag
	auto groupCount = group.size();

	world.removeComponent<PlayerTag>(2);
	auto groupAccurateCount = group.size(ComputationType::OWNED_AND_VIEWED);

	std::cout << "Removed PlayerTag from entity 2\n";
	group.each([](uint32_t entityID, Health& h)
		{
			h.hlth += 1.23f;
			std::cout << "entityID: " << entityID << ", h.hlth: " << h.hlth << "\n";
		});

	std::cout << "Group: Has? " << std::boolalpha << groupHas << ", Position " << groupHealth.hlth << ", Count: " << groupCount << ", vs: " << groupAccurateCount << "\n";


	auto groupExclusion = group.exclude<PlayerTag>();

	groupExclusion.each([](Health& h)
		{
			h.hlth -= 1.05f;
		});

	groupExclusion.each([](uint32_t entityID)
		{
			std::cout << entityID << ", ";
		});
	std::cout << "\n";

	groupExclusion.each([](uint32_t entityID, Health& h)
		{
			h.hlth -= 1.23f;
			std::cout << "entityID: " << entityID << ", h.hlth " << h.hlth << "\n";
		});

	bool groupE_Has = groupExclusion.contains(0);
	auto& groupE_vel = groupExclusion.get<Health>(0);
	auto groupE_count = groupExclusion.size();
	bool groupE_empty = groupExclusion.empty();
	auto groupE_accurateCount = groupExclusion.size(ComputationType::OWNED_AND_VIEWED);

	std::cout << "GroupExclusion: Has? " << std::boolalpha << groupE_Has << ", is empty? " << groupE_empty << ", Velocity " << groupE_vel.hlth << ", Count: " << groupE_count << ", vs: " << groupE_accurateCount << "\n";

	/* Ownership overlap throws */
	//auto groupFail = Group<Health, Velocity>(world); //Group<Health, Stamina> group(world);]

}
