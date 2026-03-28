
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

int main()
{
	World world;

	world.createEntities(5); //0-4
	world.registerComponent<Health>();
	world.registerComponent<PlayerTag>();


	world.addComponents<Health, PlayerTag>({ 0, 1, 2});

	auto view = world.view<Health, PlayerTag>(); //register view

	auto statusLambda = [&view](uint32_t entityID)
		{
			std::cout << "EntityID " << entityID << " has Health and PlayerTag: " << std::boolalpha << view.contains(entityID) << "\n";
		};

	auto halfLambda = [](Health& h)
		{
			h.hlth /= 2;
		};
	auto printLambda = [](uint32_t entityID, Health& h)
		{
			std::cout << "ID: " << entityID << ", Health: " << h.hlth << "\n";
		};

	view.each(statusLambda);

	//Entity 2 was originally in the view, but has been removed due to loss of necessary Component Stamina
	world.removeComponent<PlayerTag>(2);
	std::cout << "\nRemoved entity2's PlayerTag Component\n\n";
	view.each(statusLambda);
	std::cout << "\n";



	//Half health and Stamina of all entities in the View
	view.each(printLambda);
	std::cout << "\n";
	view.each(halfLambda);
	view.each(printLambda);
	std::cout << "\n";



	// Get assumes user-responsibility that the entityID passed IS in the view. If not a OOB index will be thrown.
	auto& h = view.get<Health>(0); //directly modify entity0's health
	h.hlth -= 35.4f;
	view.each(printLambda);
	std::cout << "\n";

	//Show how many entities are in the internal Vector of Health and Stamina
	DebugFunctions::Access::view_sparseSet_dense<Health>(world);
	DebugFunctions::Access::view_sparseSet_dense<PlayerTag>(world);

	//View recalculates the smallest Component on every "each" call. O(n) operation for however many Components the view contains
	std::cout << "Size of the Component in the View with the least entities: " << view.size() << "\n";
	//Check if View has any entities. Checks if internal smallest_Component's size is 0
	std::cout << "View is empty? " << std::boolalpha << view.empty() << "\n";


}
