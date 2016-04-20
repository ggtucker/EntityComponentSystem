#pragma once

#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <memory>
#include "Component.h"
#include "TEntitySystem.h"

template <typename TSettings>
class TEntityPrototype {
private:
	using Settings = TSettings;
	using ComponentList = typename Settings::ComponentList;

	std::unordered_map<std::type_index, std::shared_ptr<Component>> m_components;

public:

	Entity CreateEntity(TEntitySystem<Settings>& entitySystem) {
		Entity e = entitySystem.CreateEntity();
		ComponentList::ForTypes([this, &e, &entitySystem](auto t) {
			if (Contains<TYPE_OF(t)>()) {
				entitySystem.AddComponent(e, Get<TYPE_OF(t)>());
			}
		});
		return e;
	}

	template <typename CType>
	void Add(CType obj) {
		static_assert(!std::is_same<CType, Component>::value, "CType must be specific component type");
		static_assert(std::is_base_of<Component, CType>::value, "CType must be derived from Component");

		m_components[std::type_index(typeid(CType))] = std::shared_ptr<CType>(new CType(obj));
	}

	template <typename CType>
	void Remove() {
		static_assert(!std::is_same<CType, Component>::value, "CType must be specific component type");
		static_assert(std::is_base_of<Component, CType>::value, "CType must be derived from Component");

		m_components.erase(std::type_index(typeid(CType)));
	}

	template <typename CType>
	CType Get() {
		static_assert(!std::is_same<CType, Component>::value, "CType must be specific component type");
		static_assert(std::is_base_of<Component, CType>::value, "CType must be derived from Component");

		auto componentIter = m_components.find(std::type_index(typeid(CType)));
		if (componentIter != m_components.end()) {
			return *(static_cast<CType*>(componentIter->second.get()));
		}
		return CType();
	}

	template <typename CType>
	bool Contains() {
		static_assert(!std::is_same<CType, Component>::value, "CType must be specific component type");
		static_assert(std::is_base_of<Component, CType>::value, "CType must be derived from Component");

		auto componentIter = m_components.find(std::type_index(typeid(CType)));
		return componentIter != m_components.end();
	}
};

