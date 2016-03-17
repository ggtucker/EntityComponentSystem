#pragma once

class Entity {
public:
	int id;

	Entity(int id) : id{ id } {}

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

namespace std {
	template <>
	struct hash<Entity> {
		std::size_t operator()(const Entity& k) const {
			return std::hash<int>()(k.id);
		}
	};
}