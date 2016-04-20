#pragma once

#include "Reflection.h"
#include "ECSSettings.h"
#include "TEntitySystem.h"
#include "PositionComponent.h"

struct C0 {};
struct C1 {};
struct C2 {};
struct C3 {};

struct T0 {};
struct T1 {};
struct T2 {};

using S0 = Refl::TypeList<>;
using S1 = Refl::TypeList<C0, C1>;
using S2 = Refl::TypeList<C0, PositionComponent, T0>;
using S3 = Refl::TypeList<C1, T0, C3, T2>;

using EntitySystem = TEntitySystem
<
	ECSSettings<
		Refl::TypeList<C0, C1, C2, C3, PositionComponent>,
		Refl::TypeList<T0, T1, T2>,
		Refl::TypeList<S0, S1, S2, S3>
	>
>;