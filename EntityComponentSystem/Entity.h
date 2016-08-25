#pragma once

namespace ecs {
	class Entity {
	public:
		std::size_t id;

		Entity() : id{ 0 } {}

		explicit Entity(std::size_t id) : id{ id } {}

		bool operator==(const Entity& other) const {
			return id == other.id;
		}

		bool operator!=(const Entity& other) const {
			return id != other.id;
		}

		bool operator<(const Entity& other) const {
			return id < other.id;
		}

		bool operator<=(const Entity& other) const {
			return id <= other.id;
		}

		bool operator>(const Entity& other) const {
			return id > other.id;
		}

		bool operator>=(const Entity& other) const {
			return id >= other.id;
		}
	};
}

namespace std {
	template <>
	struct hash<ecs::Entity> {
		size_t operator()(const ecs::Entity& k) const {
			return hash<size_t>()(k.id);
		}
	};
}