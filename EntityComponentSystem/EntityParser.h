#pragma once

#include "json/json.h"
#include <iostream>
#include <typeindex>
#include "TEntityPrototype.h"
#include "Component.h"

namespace ecs {
	namespace EntityParser {

		template <typename TSettings>
		static TEntityPrototype<TSettings> CreatePrototype(const std::string& name, Json::Value root) {
			TEntityPrototype<TSettings> proto(name);

			std::vector<std::string> components = root.getMemberNames();

			using ComponentList = typename TSettings::ComponentList;
			ComponentList::ForTypes([&proto, &root, &components](auto t) {
				TYPE_OF(t) component;
				std::string componentName = component.Name();
				auto foundIter = std::find(components.begin(), components.end(), componentName);
				if (foundIter != components.end()) {
					component.Deserialize(root.get(componentName, ""));
					proto.Add(component);
				}
			});

			return proto;
		}

		template <typename TSettings>
		static std::vector<TEntityPrototype<TSettings>> ParseTypes(Json::Value root) {

			std::vector<TEntityPrototype<TSettings>> prototypes;

			std::vector<std::string> entityNames = root.getMemberNames();
			for (std::size_t i = 0; i < entityNames.size(); ++i) {
				Json::Value typeRoot = root.get(entityNames[i], "");
				if (typeRoot.isObject()) {
					prototypes.push_back(CreatePrototype<TSettings>(entityNames[i], typeRoot));
				}
			}

			return prototypes;
		}

	}
}