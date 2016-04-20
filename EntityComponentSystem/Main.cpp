#include <iostream>
#include "ECSTests.h"
#include "Typedefs.h"
#include "PrintSystem.h"
#include "EntityParser.h"

int main() {

	Tests::RuntimeTests();

	EntitySystem system;

	PrintSystem(system);

	std::vector<EntityPrototype> entityTypes = EntityParser::ParseTypes();

	using Settings = typename EntitySystem::Settings;
	using ComponentList = typename Settings::ComponentList;

	for (int i = 0; i < entityTypes.size(); ++i) {
		EntityPrototype& type = entityTypes[i];

		Entity e = type.CreateEntity(system);
		
		ComponentList::ForTypes([&system, &e](auto t) {
			if (system.HasComponent<TYPE_OF(t)>(e)) {
				std::cout << "Has component #" << Settings::ComponentBit<TYPE_OF(t)>() << std::endl;
			}
		});

	}



}