#include <iostream>
#include "ECSTests.h"
#include "Typedefs.h"
#include "PrintSystem.h"

int main() {

	Tests::RuntimeTests();

	EntitySystem system;

	PrintSystem(system);

}