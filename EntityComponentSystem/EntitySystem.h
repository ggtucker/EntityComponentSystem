#pragma once

#include <typeindex>
#include <typeinfo>
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "ComponentManager.h"
#include "Entity.h"

class EntitySystem {
// Type definitions
private:
	using ComponentIndexTable = std::unordered_map<Entity, unsigned int>;
	using ComponentIndexTablePtr = std::unique_ptr<ComponentIndexTable>;
	using ComponentIndexDatabase = std::unordered_map<std::type_index, ComponentIndexTablePtr>;

// Core functionality
public:

	Entity CreateEntity() {
		static int nextId = 0;
		Entity entity(nextId++);
		m_entities.push_back(entity);
		return entity;
	}

	void DeleteEntity(Entity e) {
		std::vector<Entity>::iterator result = std::find(m_entities.begin(), m_entities.end(), e);
		if (result != m_entities.end()) {
			m_entities.erase(result);
		}
	}

	template<typename ComponentType>
	void AddComponent(Entity e, ComponentType component) {
		int index = m_componentManager.AddComponent(component);
		ComponentIndexTable& table = GetComponentIndexTable<ComponentType>();
		table[e] = index;
	}

	template<typename ComponentType>
	ComponentType* GetComponent(Entity e) {
		ComponentIndexTable& table = GetComponentIndexTable<ComponentType>();
		
		ComponentIndexTable::iterator indexResult = table.find(e);
		if (indexResult != table.end()) {
			int index = indexResult->second;
			return &m_componentManager.GetComponent<ComponentType>(index);
		}

		return nullptr;
	}

private:

	template<typename ComponentType>
	ComponentIndexTable& GetComponentIndexTable() {
		std::type_index type(typeid(ComponentType));

		ComponentIndexDatabase::iterator tableResult = m_indexDatabase.find(type);
		if (tableResult == m_indexDatabase.end()) {
			m_indexDatabase[type] = std::unique_ptr<ComponentIndexTable>(new ComponentIndexTable);
			return *m_indexDatabase[type];
		}
		return *tableResult->second;
	}

// Members
private:
	ComponentIndexDatabase m_indexDatabase;
	ComponentManager m_componentManager;
	std::vector<Entity> m_entities;
};