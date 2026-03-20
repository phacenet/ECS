
#include "Core.h"

#include "World.h"
#include "DebugFunctions.h"
#include "Observer.h"
#include "Sink.h"
#include "Signal.h"
#include "Handler.h"
#include "Dispatcher.h"


struct Health
{
	float health = 100.0f;
};

struct Stamina
{
	float stamina = 100.0f;
};

struct emptyComponent
{ };

class PlayerTag : public TagBase {};


struct EntityDestroyedEvent
{
	uint32_t entityID;
};

void onEntityDestroyed(const EntityDestroyedEvent& e)
{
	std::cout << "Entity destroyed " << e.entityID << "\n";
}

int main()
{
	/* General Initialization */
	/* ========================================================= */
	World world;

	auto entity0 = world.createEntity();
	auto entity1 = world.createEntity();
	auto entity2 = world.createEntity();
	auto entity3 = world.createEntity();
	auto entity4 = world.createEntity();
	//std::vector<uint32_t> fiveEntities = createEntities(5);

	//addComponent will lazy register as well, no need to register all ahead of time
	world.registerComponent<Health>();
	world.registerComponent<Stamina>();
	world.registerComponent<PlayerTag>();
	/* ========================================================= */


	/* Debug World Functionality */
	/* ========================================================= */
	DebugFunctions::Access::view_all_IDs(world);
	DebugFunctions::Access::view_next_freeID(world);
	DebugFunctions::Access::view_all_components(world);
	/* ========================================================= */


	/* Observer */
	/*
		For monitoring when a Component is added or removed from an entity. 
		Internally stores vector of entities that have had the component
		added or removed since the observer was created.

		Can only observe one component per call currently. Need to add ability
		to take variadic pack: obs->observeAdd<Health, Stamina>(), etc.
	*/
	/* ========================================================= */
	Observer* obs = new Observer(world);
	obs->observeAdd<Health>();
	obs->observeRemove<Health>();

	world.addComponents<Health, Stamina>({ entity0, entity1 });
	world.addComponent<Health>(entity2);

	// Run passed lambda on each entity contained in internal vector since observer's inception
	obs->each([&world](uint32_t entityID)
		{
			auto& health = world.getComponentData<Health>(entityID);
			health.health -= 10.0f;
		});

	//Clear internal vector to prevent repeat
	obs->clear();

	//Internal list is currently empty
	assert(obs->size() == 0);

	world.removeComponent<Health>(entity0);
	assert(obs->size() == 1);
	obs->each([&world](uint32_t entityID)
		{
			std::cout << "Entity" << entityID << " lost its Health parameter\n";
		});

	/* ========================================================= */


	auto view = world.view<Health, Stamina>();
	std::cout << "Contains both entity1 and entity2: " << std::boolalpha << view.contains(entity1) << ", " << view.contains(entity2) << "\n";

	auto viewLambda = [](Health& h, Stamina& s)
		{
			h.health /= 2;
			s.stamina /= 2;
		};
	auto printLambda = [](uint32_t entityID, Health& h, Stamina& s)
		{
			std::cout << "ID: " << entityID << ", Health: " << h.health << ", Stamina: " << s.stamina << "\n";
		};

	view.each(viewLambda);
	view.each(printLambda);

	auto& h = view.get<Health>(entity2);
	h.health -= 10.0f;
	view.each(printLambda);

	std::cout << view.size() << "\n";
	DebugFunctions::Access::view_dense_size<Health>(world);
	std::cout << std::boolalpha << view.empty();

	/*
	Signal<void(int)> signal;
	Sink<void(int)> sink{ &signal };
	sink.connect<&onEntityDestroyed>();
	signal.publish(42);
	sink.disconnect<&onEntityDestroyed>();
	*/

	Dispatcher dispatcher;
	dispatcher.sink<EntityDestroyedEvent>().connect<&onEntityDestroyed>();

	//immediate fire
	dispatcher.trigger(EntityDestroyedEvent{ 42 });

	//deferred
	dispatcher.enqueue(EntityDestroyedEvent{ 99 });

	dispatcher.update();
}