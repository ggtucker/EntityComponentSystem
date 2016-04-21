#pragma once

#include <unordered_map>
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

	std::string m_name;
	std::unordered_map<std::type_index, std::shared_ptr<Component>> m_components;

public:

	TEntityPrototype(const std::string& name) : m_name{ name } {}

	Entity CreateEntity(TEntitySystem<Settings>& entitySystem) {
		Entity e = entitySystem.CreateEntity();
		ComponentList::ForTypes([this, &e, &entitySystem](auto t) {
			if (Contains<TYPE_OF(t)>()) {
				entitySystem.AddComponent(e, Get<TYPE_OF(t)>());
			}
		});
		return e;
	}

	const std::string& GetName() const {
		return m_name;
	}

	template <typename CType>
	void Add(CType obj) {
		static_assert(Settings::template IsComponent<CType>(), "");

		m_components[std::type_index(typeid(CType))] = std::shared_ptr<CType>(new CType(obj));
	}

	template <typename CType>
	void Remove() {
		static_assert(Settings::template IsComponent<CType>(), "");

		m_components.erase(std::type_index(typeid(CType)));
	}

	template <typename CType>
	CType Get() const {
		static_assert(Settings::template IsComponent<CType>(), "");

		auto componentIter = m_components.find(std::type_index(typeid(CType)));
		if (componentIter != m_components.end()) {
			return *(static_cast<CType*>(componentIter->second.get()));
		}
		return CType();
	}

	template <typename CType>
	bool Contains() const {
		static_assert(Settings::template IsComponent<CType>(), "");

		auto componentIter = m_components.find(std::type_index(typeid(CType)));
		return componentIter != m_components.end();
	}
};

