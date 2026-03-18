
#include "Core.h"

#include "World.h"
#include "DebugFunctions.h"
#include "Observer.h"


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

int main()
{
	World world;

	auto entity0 = world.createEntity();
	auto entity1 = world.createEntity();
	auto entity2 = world.createEntity();
	auto entity3 = world.createEntity();
	auto entity4 = world.createEntity();

	world.registerComponent<Health>();
	world.registerComponent<Stamina>();
	world.registerComponent<PlayerTag>();

	DebugFunctions::Access::view_all_IDs(world);
	DebugFunctions::Access::view_next_freeID(world);
	DebugFunctions::Access::view_all_components(world);

	Observer* obs = new Observer(world);
	obs->observeAdd<Health>();

	world.addComponents<Health, Stamina>({ entity1, entity2 });
	world.addComponent<Health>(entity3);

	obs->each([&world](uint32_t entityID)
		{
			auto& health = world.getComponentData<Health>(entityID);
			health.health -= 10.0f;
		}
	);

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

	auto& h = view.get<Health>(entity1);
	h.health -= 10.0f;
	view.each(printLambda);

	std::cout << view.size() << "\n";
	DebugFunctions::Access::view_dense_size<Health>(world);

	std::cout << std::boolalpha << view.empty();
}