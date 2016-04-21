#pragma once

#include <cassert>
#include <iostream>
#include "Reflection.h"
#include "ECSSettings.h"
#include "TEntitySystem.h"
#include "EntityParser.h"

namespace ECSTests {

	struct PositionComponent : public Component {
		float x;
		float y;
		float z;

		std::string Name() const { return "positionComponent"; }
		void Serialize(Json::Value& root) const {};
		void Deserialize(const Json::Value& root) {
			x = root.get("x", "-1").asFloat();
			y = root.get("y", "-1").asFloat();
			z = root.get("z", "-1").asFloat();
		};
	};

	struct HealthComponent : public Component {
		float health;
		float maxHealth;

		std::string Name() const { return "healthComponent"; }
		void Serialize(Json::Value& root) const {};
		void Deserialize(const Json::Value& root) {
			health = root.get("health", "-1").asFloat();
			maxHealth = root.get("maxHealth", "-1").asFloat();
		};
	};

	struct RenderableComponent : public Component {
		int meshId;

		std::string Name() const { return "renderableComponent"; }
		void Serialize(Json::Value& root) const {};
		void Deserialize(const Json::Value& root) {
			meshId = root.get("meshId", "-1").asInt();
		};
	};

	using MyComponentList = Refl::TypeList<PositionComponent, HealthComponent, RenderableComponent>;

	struct T0 {};
	struct T1 {};
	struct T2 {};
	using MyTagList = Refl::TypeList<T0, T1, T2>;

	using S0 = Refl::TypeList<>;
	using S1 = Refl::TypeList<PositionComponent, HealthComponent>;
	using S2 = Refl::TypeList<PositionComponent, RenderableComponent, T0>;
	using S3 = Refl::TypeList<HealthComponent, T0, RenderableComponent, T2>;
	using MySignatureList = Refl::TypeList<S0, S1, S2, S3>;

	using MySettings = ECSSettings<MyComponentList, MyTagList, MySignatureList>;

	static_assert(MySettings::ComponentCount == 3, "");
	static_assert(MySettings::TagCount == 3, "");
	static_assert(MySettings::SignatureCount == 4, "");

	static_assert(MySettings::ComponentId<PositionComponent>() == 0, "");
	static_assert(MySettings::ComponentId<HealthComponent>() == 1, "");
	static_assert(MySettings::ComponentId<RenderableComponent>() == 2, "");

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
		Refl::TypeList<HealthComponent, RenderableComponent>
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

		assert(bS0 == Bitset{ "000000" });
		assert(bS1 == Bitset{ "000011" });
		assert(bS2 == Bitset{ "001101" });
		assert(bS3 == Bitset{ "101110" });

		std::cout << "Signature bitset tests passed!" << std::endl;

		using EntitySystem = TEntitySystem<MySettings>;
		using EntityPrototype = TEntityPrototype<MySettings>;

		EntitySystem entitySystem;

		// Test component insertion, modification, and deletion

		Entity e = entitySystem.CreateEntity();

		PositionComponent p0;
		p0.x = 12;
		p0.y = 32;
		p0.z = 50;

		entitySystem.AddComponent(e, p0);

		PositionComponent& p1 = entitySystem.GetComponent<PositionComponent>(e);
		assert(p1.x == 12 && p1.y == 32 && p1.z == 50);

		p1.y = 101;

		PositionComponent& p2 = entitySystem.GetComponent<PositionComponent>(e);
		assert(p2.x == 12 && p2.y == 101 && p2.z == 50);

		bool removed = entitySystem.RemoveComponent<PositionComponent>(e);
		assert(removed);

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

		assert(entitySystem.IsHandleValid(e2) == false);

		PositionComponent& p4 = entitySystem.GetComponent<PositionComponent>(e4);
		assert(p4.x == 12 && p4.y == 32 && p4.z == 50);

		std::cout << "Entity removal tests passed!" << std::endl;

		// Check signature matching

		Entity e5 = entitySystem.CreateEntity();
		entitySystem.AddComponent(e5, PositionComponent());
		entitySystem.AddComponent(e5, HealthComponent());
		entitySystem.AddComponent(e5, p0);
		entitySystem.Refresh();
		assert(entitySystem.MatchesSignature<S0>(e5));
		assert(entitySystem.MatchesSignature<S1>(e5));
		assert(entitySystem.MatchesSignature<S2>(e5) == false);
		assert(entitySystem.MatchesSignature<S3>(e5) == false);

		std::cout << "Signature matching tests passed!" << std::endl;

		// Entity iteration

		entitySystem.ForEntitiesMatching<S1>([](EntityIndex i, PositionComponent& c1, HealthComponent& c2) {
			std::cout << "entity " << i << std::endl;
		});

		// Test entity prototype generation

		EntitySystem system;

		std::string entityData =
			"{\
				\"entities\": {\
					\"cow\": {\
						\"healthComponent\": {\
							\"health\": 40,\
							\"maxHealth\": 50\
						},\
						\"positionComponent\": {\
							\"x\": 5,\
							\"y\": 10,\
							\"z\": 15\
						}\
					}\
				}\
			}";
		std::istringstream inputStream(entityData, std::istream::binary);
		std::vector<EntityPrototype> entityTypes = EntityParser::ParseTypes<MySettings>(inputStream);

		for (std::size_t i = 0; i < entityTypes.size(); ++i) {
			EntityPrototype& type = entityTypes[i];

			Entity e = type.CreateEntity(system);
			
			std::cout << type.GetName() << " should have component #0, #1" << std::endl;
			MyComponentList::ForTypes([&system, &e](auto t) {
				if (system.HasComponent<TYPE_OF(t)>(e)) {
					std::cout << "  has component #" << MySettings::ComponentBit<TYPE_OF(t)>() << std::endl;
				}
			});

		}

		std::cout << "Runtime tests passed!" << std::endl;
	}
}