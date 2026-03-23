
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


void printStart(const char* section)
{
	std::cout << section << "\n===================================================================\n";
}
void printEnd()
{
	std::cout << "===================================================================\n\n";
}

/* Main Components */
/* ========================================================= */
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

/* ========================================================= */


/* Signal/Sink functions/events */
/* ========================================================= */
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
/* ========================================================= */


/* Dispatcher events/functions */
/* ========================================================= */
struct EntityDestroyedEvent
{
	uint32_t entityID;
};

void onEntityDestroyed(const EntityDestroyedEvent& e)
{
	std::cout << "Entity destroyed " << e.entityID << "\n";
}

struct deleteEntityEvent
{
	World* world;
	uint32_t entityID;
};

void deleteEntity(const deleteEntityEvent& de)
{
	de.world->destroyEntity(de.entityID);
	std::cout << "Destroyed entity " << de.entityID << "\n";
}
/* ========================================================= */


int main()
{
	/* General Initialization */
	/* ========================================================= */
	World world;

	printStart("Creation and DEBUG:");
	auto entity0 = world.createEntity();
	auto entity1 = world.createEntity();
	auto entity2 = world.createEntity();
	auto entity3 = world.createEntity();
	auto entity4 = world.createEntity();
	std::vector<uint32_t> entities{ entity0, entity1, entity2, entity3, entity4 };
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
	printEnd();
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
	printStart("Observer:");
	Observer* obs = new Observer(world);
	obs->observeAdd<Health>();
	obs->observeRemove<Health>();

	world.addComponents<Health, Stamina>({ entity0, entity1 });
	world.addComponent<Health>(entity2);
	std::cout << obs->size() << " pending entityIDs inside of obs\n";

	// Run passed lambda on each entity contained in internal vector since observer's inception (3 pending IDs).
	obs->each([&world](uint32_t entityID)
		{
			auto& health = world.getComponentData<Health>(entityID);
			std::cout << "Health parameter for entity" << entityID << " BEFORE each() modification: " << health.health << "\n";
			health.health -= 10.0f;
			std::cout << "Health parameter for entity" << entityID << " AFTER each() modification: " << health.health << "\n";
		});

	//Clear internal vector to prevent repeat runs on same entityIDs
	obs->clear();
	//Internal list is currently empty
	assert(obs->size() == 0);

	//Add entity0's ID to the internal list for observeRemove<Health>
	world.removeComponent<Health>(entity0);
	assert(obs->size() == 1);

	obs->each([&world](uint32_t entityID)
		{
			std::cout << "Entity" << entityID << " lost its Health parameter\n";
		});
	DebugFunctions::Access::view_SparseSet_Observers<Health>(world, Observer::ObserverType::BOTH);
	
	obs->unregister<Health>(Observer::ObserverType::ONADD);
	DebugFunctions::Access::view_SparseSet_Observers<Health>(world, Observer::ObserverType::BOTH);


	printEnd();
	/* ========================================================= */


	/* Reset entities */
	/* ========================================================= */
	auto viewHealth = world.view<Health>();
	auto viewStamina = world.view<Stamina>();

	auto removeHealth = [&](uint32_t entityID)
		{
			if (viewHealth.contains(entityID))
				world.removeComponent<Health>(entityID);
		};

	auto removeStamina = [&](uint32_t entityID)
		{
			if (viewStamina.contains(entityID))
				world.removeComponent<Stamina>(entityID);
		};

	for (const auto& entity : entities)
	{
		viewHealth.each(removeHealth);
		viewHealth.each(removeStamina);
	}
	/* ========================================================= */


	/* View 
		* Viewing "groups" of entities that ALL own the specified Components.
		* Views are for operating on the aforementioned entities or ensuring 
			entity status is still as expected after changes.
		* Can also be used to grab all entities that have any component type
			and quickly remove the Component from all entities.
	*/
	/* ========================================================= */
	printStart("View:");

	world.addComponents<Health, Stamina>({ entity0, entity1, entity2 });

	auto view = world.view<Health, Stamina>(); //register view

	auto statusLambda = [&view](uint32_t entityID)
		{
			std::cout << "EntityID " << entityID << " has Health and Stamina: " << std::boolalpha << view.contains(entityID) << "\n";
		};
	auto halfLambda = [](Health& h, Stamina& s)
		{
			h.health /= 2;
			s.stamina /= 2;
		};
	auto printLambda = [](uint32_t entityID, Health& h, Stamina& s)
		{
			std::cout << "ID: " << entityID << ", Health: " << h.health << ", Stamina: " << s.stamina << "\n";
		};

	view.each(statusLambda);

	//Entity 2 was originally in the view, but has been removed due to loss of necessary Component Stamina
	world.removeComponent<Stamina>(entity2);
	std::cout << "\nRemoved entity2's Stamina Component\n\n";
	view.each(statusLambda);
	std::cout << "\n";

	//Half health and Stamina of all entities in the View
	view.each(printLambda);
	std::cout << "\n";
	view.each(halfLambda);
	view.each(printLambda);
	std::cout << "\n";

	// Get assumes user-responsibility that the entityID passed IS in the view. If not a OOB index will be thrown.
	auto& h = view.get<Health>(entity0); //directly modify entity0's health
	h.health -= 35.4f;
	view.each(printLambda);
	std::cout << "\n";

	//Show how many entities are in the internal Vector of Health and Stamina
	DebugFunctions::Access::view_sparseSet_dense<Health>(world);
	DebugFunctions::Access::view_sparseSet_dense<Stamina>(world);
	
	//View recalculates the smallest Component on every "each" call. O(n) operation for however many Components the view contains
	std::cout << "Size of the Component in the View with the least entities: " << view.size() << "\n";
	//Check if View has any entities. Checks if internal smallest_Component's size is 0
	std::cout << "View is empty? " << std::boolalpha << view.empty() << "\n";
	printEnd();
	/* ========================================================= */


	/* Manual Signal/Sink */
	/*
		* Used for attaching some data, an "Event" to an arbitrary function, and firing it manually
		* Signal is the bookeeping class, and sink is the public interface
		* Useful for manually firing multiple stored functions at once
		* Essentially, the event is the params for the function to operate on, similar to a lambda:
		* [](event){do stuff};
	*/
	/* ========================================================= */
	printStart("Signal/Sink:");
	HealthSystem healthSystem; //Must register actual object of type to call method for member functions

	/* Register and connect a signal via public interface sink and link them */
	Signal<void(const DamageEvent&)> signal; //<return type (params)> of function(s) to call
	Sink<void(const DamageEvent&)> sink{ &signal };

	/* Connect the function HealthSystem::onDamage on the healthSystem instance */
	sink.connect<&HealthSystem::onDamage>(&healthSystem);
	signal.publish(DamageEvent{0, 25.0f});
	sink.disconnect<&HealthSystem::onDamage>(&healthSystem);
	std::cout << "\n";

	/* Or for a free function/static method. Can also connect multiple functions to be run once publish is called */
	sink.connect<&freeOnDamage>();
	sink.connect<&freeAddHealth>();
	signal.publish(DamageEvent{ 1, 25.0f });
	std::cout << "\n";

	/* Disconnect one event, but keep and run the other */
	signal.disconnect<&freeOnDamage>();
	signal.publish(DamageEvent{ 1, 25.0f });
	signal.disconnect<&freeAddHealth>();

	assert(signal.empty()); //after disconnect, signal should have no callbacks or tokens internally
	printEnd();
	/* ========================================================= */


	/* Dispatcher 
		* Automatically create and manage signals and sinks
		* Provides additional functionality for queueing Events
		* Events are the data to be passed to the registered function
		* Does not support lambdas with captures. Must use Event struct to pass "captured" data, like World*
		* The struct is the "context carrier"
	*/
	/* ========================================================= */
	printStart("Dispatcher:");

	/* Connect a listener via a sink */
	Dispatcher dispatcher;
	dispatcher.sink<EntityDestroyedEvent>().connect<&onEntityDestroyed>(); //<Event (data)>...<Function>

	/* Fire Event immediately with ID 42 */
	dispatcher.trigger(EntityDestroyedEvent{ 42 });

	/* Place Event in the queue, will be fired on update with ID 99 */
	dispatcher.enqueue(EntityDestroyedEvent{ 99 });
	dispatcher.enqueue(EntityDestroyedEvent{ 55 });

	/* Fire all queued Events */
	dispatcher.update();

	/* Remove all Event data for that Event */
	dispatcher.clear<EntityDestroyedEvent>();
	dispatcher.update(); //does nothing
	std::cout << "\n";

	/* Can "capture" data using the Event struct */
	Dispatcher d;
	d.sink<deleteEntityEvent>().connect<&deleteEntity>();
	d.enqueue(deleteEntityEvent{ &world, entity0 });
	d.enqueue(deleteEntityEvent{ &world, entity1 });
	d.update();
	DebugFunctions::Access::view_all_IDs(world);

	printEnd();
	/* ========================================================= */

	/* Groups */

	Group<Health, Stamina> group(world);
	GroupObserver<Health, Stamina> groupObserver(group);
}