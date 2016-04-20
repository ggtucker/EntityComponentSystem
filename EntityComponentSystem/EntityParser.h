#pragma once

#include "Typedefs.h"
#include "json/json.h"
#include <fstream>
#include <iostream>
#include "Component.h"

namespace EntityParser {

	static bool contains_field(std::vector<std::string>& entityFields, std::string&& field) {
		return std::find(entityFields.begin(), entityFields.end(), field) != entityFields.end();
	}

	static EntityPrototype CreatePrototype(Json::Value root) {
		EntityPrototype proto;
		std::vector<std::string> components = root.getMemberNames();
		for (std::size_t i = 0; i < components.size(); ++i) {
			std::cout << components[i] << std::endl;
			Json::Value componentRoot = root.get(components[i], "");
			if (components[i] == "healthComponent") {
				HealthComponent component;
				component.health = componentRoot.get("health", "-1").asFloat();
				component.maxHealth = componentRoot.get("maxHealth", "-1").asFloat();
				proto.Add(component);
			} else if(components[i] == "positionComponent") {
				PositionComponent component;
				component.x = componentRoot.get("x", "-1").asFloat();
				component.y = componentRoot.get("y", "-1").asFloat();
				component.z = componentRoot.get("z", "-1").asFloat();
				proto.Add(component);
			}
		}
		return proto;
	}

	static std::vector<EntityPrototype> ParseTypes() {

		std::vector<EntityPrototype> prototypes;

		Json::Value root;
		Json::Reader reader;
		std::ifstream test("entity_data.txt", std::ifstream::binary);
		bool success = reader.parse(test, root, false);
		if (!success) {
			std::cout << reader.getFormattedErrorMessages() << std::endl;
		}
		root = root.get("entities", "");
		if (root.isObject()) {
			std::vector<std::string> entityNames = root.getMemberNames();
			for (std::size_t i = 0; i < entityNames.size(); ++i) {
				Json::Value typeRoot = root.get(entityNames[i], "");
				if (typeRoot.isObject()) {
					prototypes.push_back(CreatePrototype(typeRoot));
				}
			}
		}

		return prototypes;
	}

}