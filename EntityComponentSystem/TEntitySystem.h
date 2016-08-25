#pragma once

#include <typeindex>
#include <typeinfo>
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "Entity.h"
#include "Component.h"
#include "Reflection.h"

namespace ecs {
	using EntityIndex = std::size_t;
	using ComponentIndex = std::size_t;

	template <typename TSettings>
	struct TEntityData {
		using Settings = TSettings;
		using Bitset = typename Settings::Bitset;

		TEntityData() {}

		bool alive;
		Entity id;
		Bitset signature;
	};

	template <typename TSettings>
	class TEntitySystem {
	public:
		using Settings = TSettings;
	private:
		using ThisType = TEntitySystem<Settings>;
		using EntityData = TEntityData<Settings>;

		using ComponentList = typename Settings::ComponentList;
		using ComponentPoolTuple = typename ComponentList::template WrapTypes<ComponentPool>::ListTuple;
		using SignatureBitsetStorage = typename Settings::SignatureBitsetStorage;

		using EntityIndexTable = std::unordered_map<Entity, EntityIndex>;
		using ComponentIndexTable = std::unordered_map<Entity, ComponentIndex>;
		using ComponentIndexDatabase = Refl::Repeat<ComponentList::Size, ComponentIndexTable>;

		SignatureBitsetStorage m_signatureBitsets;

		ComponentIndexDatabase m_componentIndexDatabase;
		ComponentPoolTuple m_componentPools;

		EntityIndexTable m_entityIndexTable;
		std::vector<EntityData> m_entities;

		std::size_t m_size{ 0 }, m_nextSize{ 0 };

		void growEntityCapacity(std::size_t newCapacity) {
			std::size_t capacity = m_entities.capacity();
			assert(newCapacity > capacity);

			m_entities.resize(newCapacity);

			for (std::size_t i = capacity; i < newCapacity; ++i) {
				EntityData& entity(m_entities[i]);
				entity.alive = false;
				entity.id = Entity();
				entity.signature.reset();
			}
		}

		void growIfNeeded() {
			if (m_entities.capacity() > m_nextSize) return;
			growEntityCapacity((m_entities.capacity() + 10) * 2);
		}

		EntityData& getEntityData(Entity e) noexcept {
			return getEntityData(getEntityIndex(e));
		}

		const EntityData& getEntityData(Entity e) const noexcept {
			return getEntityData(getEntityIndex(e));
		}

		EntityData& getEntityData(EntityIndex index) noexcept {
			assert(m_nextSize > index);
			return m_entities[index];
		}

		const EntityData& getEntityData(EntityIndex index) const noexcept {
			assert(m_nextSize > index);
			return m_entities[index];
		}

		void deleteEntity(EntityIndex index) noexcept {
			EntityData& entity = getEntityData(index);
			RemoveAllComponents(entity.id);
			m_entityIndexTable.erase(entity.id);
			entity.id = Entity();
			entity.alive = false;
		}

	public:

		TEntitySystem() { growEntityCapacity(100); }

		Entity CreateEntity() {
			static std::size_t idCounter = 1;

			growIfNeeded();
			EntityIndex freeIndex(m_nextSize++);
			assert(!IsAlive(freeIndex));

			EntityData& entity(m_entities[freeIndex]);
			entity.alive = true;
			entity.id = Entity(idCounter++);
			entity.signature.reset();
			m_entityIndexTable[entity.id] = freeIndex;

			assert(IsHandleValid(entity.id));
			return entity.id;
		}

		EntityIndex& getEntityIndex(Entity e) noexcept {
			assert(IsHandleValid(e));
			return m_entityIndexTable[e];
		}

		const EntityIndex& getEntityIndex(Entity e) const noexcept {
			assert(IsHandleValid(e));
			return m_entityIndexTable.at(e);
		}

		bool IsHandleValid(Entity e) const noexcept {
			return m_entityIndexTable.find(e) != m_entityIndexTable.end();
		}

		bool IsAlive(EntityIndex index) const noexcept {
			return getEntityData(index).alive;
		}

		bool IsAlive(Entity e) const noexcept {
			return IsAlive(getEntityIndex(e));
		}

		void Kill(EntityIndex index) noexcept {
			getEntityData(index).alive = false;
		}

		void Kill(Entity e) noexcept {
			Kill(getEntityIndex(e));
		}

		template <typename TagType>
		bool HasTag(EntityIndex index) const noexcept {
			static_assert(Settings::template IsTag<TagType>(), "");
			return getEntityData(index).signature[Settings::template TagBit<TagType>()];
		}

		template <typename TagType>
		bool HasTag(Entity e) const noexcept {
			return HasTag<TagType>(getEntityIndex(e));
		}

		template <typename TagType>
		void AddTag(EntityIndex index) noexcept {
			static_assert(Settings::template IsTag<TagType>(), "");
			getEntityData(index).signature[Settings::template TagBit<TagType>()] = true;
		}

		template <typename TagType>
		void AddTag(Entity e) noexcept {
			AddTag<TagType>(getEntityIndex(e));
		}

		template <typename TagType>
		void RemoveTag(EntityIndex index) noexcept {
			static_assert(Settings::template IsTag<TagType>(), "");
			getEntityData(index).signature[Settings::template TagBit<TagType>()] = false;
		}

		template <typename TagType>
		void RemoveTag(Entity e) noexcept {
			RemoveTag<TagType>(getEntityIndex(e));
		}

		template <typename ComponentType>
		bool HasComponent(EntityIndex index) const noexcept {
			static_assert(Settings::template IsComponent<ComponentType>(), "");

			return getEntityData(index).signature[Settings::template ComponentBit<ComponentType>()];
		}

		template <typename ComponentType>
		bool HasComponent(Entity e) const noexcept {
			return HasComponent<ComponentType>(getEntityIndex(e));
		}

		template <typename ComponentType>
		void AddComponent(Entity e, const ComponentType& component) noexcept {
			static_assert(Settings::template IsComponent<ComponentType>(), "");

			ComponentIndexTable& table = getComponentIndexTable<ComponentType>();
			ComponentPool<ComponentType>& pool = getComponentPool<ComponentType>();

			table[e] = pool.create(component);
			getEntityData(e).signature[Settings::template ComponentBit<ComponentType>()] = true;
		}

		template <typename ComponentType>
		ComponentType& GetComponent(Entity e) noexcept {
			static_assert(Settings::template IsComponent<ComponentType>(), "");
			assert(HasComponent<ComponentType>(e));

			ComponentPool<ComponentType>& pool = getComponentPool<ComponentType>();

			ComponentIndex index;
			getComponentIndex<ComponentType>(e, index);
			return pool[index];
		}

		void RemoveAllComponents(Entity e) noexcept {
			ComponentList::ForTypes([this, &e](auto t) {
				RemoveComponent<TYPE_OF(t)>(e);
			});
		}

		template <typename ComponentType>
		bool RemoveComponent(Entity e) noexcept {
			static_assert(Settings::template IsComponent<ComponentType>(), "");

			ComponentIndexTable& table = getComponentIndexTable<ComponentType>();
			ComponentPool<ComponentType>& pool = getComponentPool<ComponentType>();

			ComponentIndex index;
			if (getComponentIndex<ComponentType>(e, index)) {
				pool.deallocate(index);
				table.erase(e);
				getEntityData(e).signature[Settings::template ComponentBit<ComponentType>()] = false;
				return true;
			}
			return false;
		}

		void Clear() noexcept {
			for (std::size_t i = 0; i < m_entities.capacity(); ++i) {
				EntityData& entity(m_entities[i]);
				if (entity.alive) {
					RemoveAllComponents(entity.id);
					entity.alive = false;
					entity.id = Entity();
				}
			}
			m_entityIndexTable.clear();
			m_size = m_nextSize = 0;
		}

		void Refresh() noexcept {
			if (m_nextSize == 0) {
				m_size = 0;
				return;
			}

			EntityIndex deadIdx{ 0 }, aliveIdx{ m_nextSize - 1 };

			while (true) {
				while (IsAlive(deadIdx)) {
					if (++deadIdx > aliveIdx) {
						m_size = m_nextSize = deadIdx;
						return;
					}
				}

				while (!IsAlive(aliveIdx)) {
					deleteEntity(aliveIdx);
					if (--aliveIdx <= deadIdx) {
						m_size = m_nextSize = deadIdx;
						return;
					}
				}

				assert(!IsAlive(deadIdx));
				assert(IsAlive(aliveIdx));

				deleteEntity(deadIdx);

				EntityData& dead = getEntityData(deadIdx);
				EntityData& alive = getEntityData(aliveIdx);

				m_entityIndexTable[alive.id] = deadIdx;
				std::swap(dead, alive);

				++deadIdx; --aliveIdx;
			}
		}

		template <typename TSignature>
		bool MatchesSignature(EntityIndex index) const noexcept {
			static_assert(Settings::template IsSignature<TSignature>(), "");

			const auto& entityBitset(getEntityData(index).signature);
			const auto& signatureBitset(m_signatureBitsets.template GetSignatureBitset<TSignature>());

			return (entityBitset & signatureBitset) == signatureBitset;
		}

		template <typename TSignature>
		bool MatchesSignature(Entity e) const noexcept {
			return MatchesSignature<TSignature>(getEntityIndex(e));
		}

		template <typename TFunc>
		void ForEntities(TFunc&& func) {
			for (EntityIndex i = 0; i < m_size; ++i) {
				func(i);
			}
		}

		template <typename TSignature, typename TFunc>
		void ForEntitiesMatching(TFunc&& func) {
			static_assert(Settings::template IsSignature<TSignature>(), "");

			for (EntityIndex i = 0; i < m_size; ++i) {
				if (MatchesSignature<TSignature>(i)) {
					expandSignatureCall<TSignature>(i, func);
				}
			}
		}

	private:

		template <typename TSignature, typename TFunc>
		void expandSignatureCall(EntityIndex index, TFunc&& func) {
			static_assert(Settings::template IsSignature<TSignature>(), "");

			using RequiredComponents = typename Settings::SignatureBitset::template SignatureComponents<TSignature>;
			using Helper = typename RequiredComponents::template Rename<ExpandCallHelper>;

			Helper::call(*this, index, func);
		}

		template <typename... Ts>
		struct ExpandCallHelper {
			template <typename TFunc>
			static void call(ThisType& entitySystem, EntityIndex index, TFunc&& func) {
				Entity entity = entitySystem.getEntityData(index).id;
				func(index, entitySystem.GetComponent<Ts>(entity)...);
			}
		};

		template <typename ComponentType>
		bool getComponentIndex(Entity e, ComponentIndex& index) {
			static_assert(Settings::template IsComponent<ComponentType>(), "");

			ComponentIndexTable& table = getComponentIndexTable<ComponentType>();
			ComponentIndexTable::iterator indexResult = table.find(e);
			if (indexResult != table.end()) {
				index = indexResult->second;
				return true;
			}
			return false;
		}

		template <typename ComponentType>
		ComponentPool<ComponentType>& getComponentPool() {
			return std::get<ComponentList::template IndexOf<ComponentType>()>(m_componentPools);
		}

		template<typename ComponentType>
		ComponentIndexTable& getComponentIndexTable() {
			return std::get<ComponentList::template IndexOf<ComponentType>()>(m_componentIndexDatabase);
		}
	};
}