#pragma once

#include "TIndexMemoryPool.h"

static constexpr std::size_t COMPONENT_POOL_SIZE = 1024U;

template <typename ComponentType>
using ComponentPool = TIndexMemoryPool<ComponentType, COMPONENT_POOL_SIZE>;

class Component {};