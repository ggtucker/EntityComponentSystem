#pragma once

#include "Typedefs.h"

static void PrintSystem(EntitySystem& entitySystem) {
	entitySystem.ForEntitiesMatching<S1>([](EntityIndex i, C0& c0, C1& c1) {
		std::cout << "entity " << i << std::endl;
	});
}