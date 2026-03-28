
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
	float health = 100.0f;
};

struct Stamina
{
	float stamina = 100.0f;
};

int main()
{
	
	World world;
	auto entity0 = world.createEntity();
	auto entity1 = world.createEntity();
	auto entity2 = world.createEntity();
	auto entity3 = world.createEntity();

	
	world.addComponents<Health, Stamina>({ entity1, entity3 });
	world.serialize<Health, Stamina>("test.bin");
	

	// then deserialize into a fresh world
	
	
	World world2;
	world2.deserialize<Health, Stamina>("test.bin");
	DebugFunctions::Access::view_dense_size<Health>(world2);
	DebugFunctions::Access::view_dense_size<Stamina>(world2);

	DebugFunctions::Access::view_all_IDs(world2);
	
}
