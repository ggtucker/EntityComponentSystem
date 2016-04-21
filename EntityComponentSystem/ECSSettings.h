#pragma once

#include <bitset>
#include "Reflection.h"
#include "Component.h"

namespace Sig {
	template<typename TSettings>
	struct SignatureBitset;

	template<typename TSettings>
	struct SignatureBitsetStorage;
}

template
<
	typename TComponentList,
	typename TTagList,
	typename TSignatureList
>
struct ECSSettings {

	using ComponentList = TComponentList;
	using TagList = TTagList;
	using SignatureList = TSignatureList;
	using ThisType = ECSSettings<ComponentList, TagList, SignatureList>;

	using SignatureBitset = Sig::SignatureBitset<ThisType>;
	using SignatureBitsetStorage = Sig::SignatureBitsetStorage<ThisType>;

	// Count of various types

	static constexpr std::size_t ComponentCount = ComponentList::Size;

	static constexpr std::size_t TagCount = TagList::Size;

	static constexpr std::size_t SignatureCount = SignatureList::Size;

	// Check type "traits"

	template <typename T>
	static constexpr bool IsComponent() noexcept {
		return ComponentList::Contains<T>()
			&& std::is_base_of<Component, T>::value
			&& !std::is_same<Component, T>::value;
	}

	template <typename T>
	static constexpr bool IsTag() noexcept {
		return TagList::Contains<T>();
	}

	template <typename T>
	static constexpr bool IsSignature() noexcept {
		return SignatureList::Contains<T>();
	}

	// Unique ID for each type

	template <typename T>
	static constexpr std::size_t ComponentId() noexcept {
		return ComponentList::IndexOf<T>();
	}

	template <typename T>
	static constexpr std::size_t TagId() noexcept {
		return TagList::IndexOf<T>();
	}

	template <typename T>
	static constexpr std::size_t SignatureId() noexcept {
		return SignatureList::IndexOf<T>();
	}

	// Bitset type and indexing

	using Bitset = std::bitset<ComponentCount + TagCount>;

	template <typename T>
	static constexpr std::size_t ComponentBit() noexcept {
		return ComponentId<T>();
	}

	template <typename T>
	static constexpr std::size_t TagBit() noexcept {
		return ComponentCount + TagId<T>();
	}
};

namespace Sig {
	template <typename TSettings>
	struct SignatureBitset {
		using Settings = TSettings;
		using ThisType = SignatureBitset<Settings>;
		using SignatureList = typename Settings::SignatureList;
		using Bitset = typename Settings::Bitset;

		using BitsetStorage = Refl::Repeat<Settings::SignatureCount, Bitset>;

		template <typename T>
		using IsComponentFilter = std::integral_constant<bool, Settings::template IsComponent<T>()>;

		template <typename T>
		using IsTagFilter = std::integral_constant<bool, Settings::template IsTag<T>()>;

		template <typename TSignature>
		using SignatureComponents = typename TSignature::template Filter<IsComponentFilter>;

		template <typename TSignature>
		using SignatureTags = typename TSignature::template Filter<IsTagFilter>;
	};

	template <typename TSettings>
	struct SignatureBitsetStorage {
	private:
		using Settings = TSettings;
		using SignatureBitset = typename Settings::SignatureBitset;
		using SignatureList = typename SignatureBitset::SignatureList;
		using BitsetStorage = typename SignatureBitset::BitsetStorage;

		BitsetStorage m_storage;

	public:
		SignatureBitsetStorage() noexcept {
			SignatureList::ForTypes([this](auto t) {
				this->InitBitset<TYPE_OF(t)>();
			});
		}

		template <typename T>
		auto& GetSignatureBitset() noexcept {
			static_assert(Settings::template IsSignature<T>(), "");

			return std::get<Settings::template SignatureId<T>()>(m_storage);
		}

		template <typename T>
		const auto& GetSignatureBitset() const noexcept {
			static_assert(Settings::template IsSignature<T>(), "");

			return std::get<Settings::template SignatureId<T>()>(m_storage);
		}

	private:
		template <typename T>
		void InitBitset() noexcept {
			auto& bitset(this->GetSignatureBitset<T>());

			using SignatureComponents = typename SignatureBitset::template SignatureComponents<T>;
			using SignatureTags = typename SignatureBitset::template SignatureTags<T>;

			SignatureComponents::ForTypes([this, &bitset](auto t) {
				bitset[Settings::template ComponentBit<TYPE_OF(t)>()] = true;
			});

			SignatureTags::ForTypes([this, &bitset](auto t) {
				bitset[Settings::template TagBit<TYPE_OF(t)>()] = true;
			});
		}
	};
}