#pragma once

#include <vector>
#include "PositionComponent.h"
#include "RenderableComponent.h"

class ComponentManager {
// Type definitions
private:
	template <typename ComponentType>
	using ComponentArray = std::vector<ComponentType>;

	struct ComponentArrays : ComponentArray<PositionComponent>,
							 ComponentArray<RenderableComponent> {};

// Core functionality
public:

	template<typename ComponentType>
	unsigned int AddComponent(ComponentType component) {
		ComponentArray<ComponentType>& componentArray = m_componentArrays;
		componentArray.push_back(component);
		return componentArray.size() - 1;
	}

	template<typename ComponentType>
	ComponentType& GetComponent(unsigned int index) {
		ComponentArray<ComponentType>& componentArray = m_componentArrays;
		return componentArray[index];
	}

	template<typename ComponentType>
	ComponentArray<ComponentType>& GetComponentArray() {
		ComponentArray<ComponentType>& componentArray = m_componentArrays;
		return componentArray;
	}
		
// Members
private:
	ComponentArrays m_componentArrays;
};