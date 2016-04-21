#pragma once

#include "json/json.h"
#include "TIndexMemoryPool.h"

static constexpr std::size_t COMPONENT_POOL_SIZE = 1024U;

template <typename ComponentType>
using ComponentPool = TIndexMemoryPool<ComponentType, COMPONENT_POOL_SIZE>;

struct Component {
	virtual std::string Name() const = 0;
	virtual void Serialize(Json::Value& root) const = 0;
	virtual void Deserialize(const Json::Value& root) = 0;
};