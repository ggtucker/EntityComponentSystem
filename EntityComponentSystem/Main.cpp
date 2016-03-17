#include <iostream>
#include "EntitySystem.h"
#include "PositionComponent.h"

int main() {
	EntitySystem entitySystem;

	Entity e = entitySystem.CreateEntity();

	PositionComponent p0;
	p0.x = 12;
	p0.y = 32;
	p0.z = 50;
	std::cout << "p0: [" << p0.x << ", " << p0.y << ", " << p0.z << "]" << std::endl;

	PositionComponent* p1 = entitySystem.GetComponent<PositionComponent>(e);
	if (p1) {
		std::cout << "p1: [" << p1->x << ", " << p1->y << ", " << p1->z << "]" << std::endl;
	}

	entitySystem.AddComponent(e, p0);

	PositionComponent* p2 = entitySystem.GetComponent<PositionComponent>(e);
	std::cout << "p2: [" << p2->x << ", " << p2->y << ", " << p2->z << "]" << std::endl;
	p2->y = 101;
	
	PositionComponent* p3 = entitySystem.GetComponent<PositionComponent>(e);
	std::cout << "p3: [" << p3->x << ", " << p3->y << ", " << p3->z << "]" << std::endl;
}