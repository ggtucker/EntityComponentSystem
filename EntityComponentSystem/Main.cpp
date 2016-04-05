#include <iostream>
#include <string>
#include <cassert>
#include <type_traits>
#include "EntitySystem.h"
#include "Reflection.h"
#include "ECSSettings.h"
#include "PositionComponent.h"

namespace Tests {

	struct C0 {};
	struct C1 {};
	struct C2 {};
	struct C3 {};
	using MyComponentList = Refl::TypeList<C0, C1, C2, C3, PositionComponent>;

	struct T0 {};
	struct T1 {};
	struct T2 {};
	using MyTagList = Refl::TypeList<T0, T1, T2>;

	using S0 = Refl::TypeList<>;
	using S1 = Refl::TypeList<C0, C1>;
	using S2 = Refl::TypeList<C0, PositionComponent, T0>;
	using S3 = Refl::TypeList<C1, T0, C3, T2>;
	using MySignatureList = Refl::TypeList<S0, S1, S2, S3>;

	using MySettings = ECSSettings<MyComponentList, MyTagList, MySignatureList>;

	static_assert(MySettings::ComponentCount == 5, "");
	static_assert(MySettings::TagCount == 3, "");
	static_assert(MySettings::SignatureCount == 4, "");

	static_assert(MySettings::ComponentId<C0>() == 0, "");
	static_assert(MySettings::ComponentId<C1>() == 1, "");
	static_assert(MySettings::ComponentId<C2>() == 2, "");
	static_assert(MySettings::ComponentId<C3>() == 3, "");
	static_assert(MySettings::ComponentId<PositionComponent>() == 4, "");

	static_assert(MySettings::TagId<T0>() == 0, "");
	static_assert(MySettings::TagId<T1>() == 1, "");
	static_assert(MySettings::TagId<T2>() == 2, "");

	static_assert(MySettings::SignatureId<S0>() == 0, "");
	static_assert(MySettings::SignatureId<S1>() == 1, "");
	static_assert(MySettings::SignatureId<S2>() == 2, "");
	static_assert(MySettings::SignatureId<S3>() == 3, "");

	using MySignatureBitset = typename MySettings::SignatureBitset;

	static_assert(std::is_same
		<
		MySignatureBitset::SignatureComponents<S0>,
		Refl::TypeList<>
		>::value, "");

	static_assert(std::is_same
		<
		MySignatureBitset::SignatureComponents<S3>,
		Refl::TypeList<C1, C3>
		>::value, "");

	static_assert(std::is_same
		<
		MySignatureBitset::SignatureTags<S3>,
		Refl::TypeList<T0, T2>
		>::value, "");

	void RuntimeTests() {
		using Bitset = typename MySignatureBitset::Bitset;
		Sig::SignatureBitsetStorage<MySettings> msb;

		const auto& bS0(msb.GetSignatureBitset<S0>());
		const auto& bS1(msb.GetSignatureBitset<S1>());
		const auto& bS2(msb.GetSignatureBitset<S2>());
		const auto& bS3(msb.GetSignatureBitset<S3>());

		assert(bS0 == Bitset{ "00000000" });
		assert(bS1 == Bitset{ "00000011" });
		assert(bS2 == Bitset{ "00110001" });
		assert(bS3 == Bitset{ "10101010" });

		std::cout << "Signature bitset tests passed!" << std::endl;

		using EntitySystem = EntitySystem<MySettings>;

		EntitySystem entitySystem;

		// Test component insertion, modification, and deletion

		Entity e = entitySystem.CreateEntity();

		PositionComponent p0;
		p0.x = 12;
		p0.y = 32;
		p0.z = 50;

		PositionComponent* p1 = entitySystem.GetComponent<PositionComponent>(e);
		assert(p1 == nullptr);

		entitySystem.AddComponent(e, p0);

		PositionComponent* p2 = entitySystem.GetComponent<PositionComponent>(e);
		assert(p2->x == 12 && p2->y == 32 && p2->z == 50);

		p2->y = 101;

		PositionComponent* p3 = entitySystem.GetComponent<PositionComponent>(e);
		assert(p3->x == 12 && p3->y == 101 && p3->z == 50);

		bool removed = entitySystem.RemoveComponent<PositionComponent>(e);
		assert(removed);
		
		PositionComponent* p4 = entitySystem.GetComponent<PositionComponent>(e);
		assert(p4 == nullptr);

		std::cout << "Component insertion, modification, and deletion tests passed!" << std::endl;

		// Test removal of entity

		Entity e1 = entitySystem.CreateEntity();
		Entity e2 = entitySystem.CreateEntity();
		entitySystem.AddComponent(e2, p0);
		Entity e3 = entitySystem.CreateEntity();
		Entity e4 = entitySystem.CreateEntity();
		entitySystem.AddComponent(e4, p0);

		entitySystem.Kill(e2);
		entitySystem.Refresh();

		PositionComponent* p5 = entitySystem.GetComponent<PositionComponent>(e2);
		assert(p5 == nullptr);

		PositionComponent* p6 = entitySystem.GetComponent<PositionComponent>(e4);
		assert(p6->x == 12 && p6->y == 32 && p6->z == 50);

		std::cout << "Entity removal tests passed!" << std::endl;

		// Check signature matching

		Entity e5 = entitySystem.CreateEntity();
		entitySystem.AddComponent(e5, C0());
		entitySystem.AddComponent(e5, C1());
		entitySystem.AddComponent(e5, p0);
		entitySystem.Refresh();
		assert(entitySystem.MatchesSignature<S0>(e5));
		assert(entitySystem.MatchesSignature<S1>(e5));
		assert(entitySystem.MatchesSignature<S2>(e5) == false);
		assert(entitySystem.MatchesSignature<S3>(e5) == false);

		std::cout << "Signature matching tests passed!" << std::endl;

		// Entity iteration

		entitySystem.ForEntitiesMatching<S1>([](auto i, auto& c1, auto& c2) {
			std::cout << "entity " << i << std::endl;
		});
	}
}

int main() {

	Tests::RuntimeTests();
	std::cout << "Runtime tests passed!" << std::endl;

}