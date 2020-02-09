#pragma once

#include <polar/core/serializer.h>

namespace polar::support::asset {
	template<typename T> class asset_ref {
		std::string _name;

	  public:
		asset_ref() = default;
		asset_ref(std::string name) : _name(name) {}

		const std::string &name() const { return _name; }

		void set(std::string name) {
			_name = name;
		}
	};

	template<typename T> inline core::serializer &operator<<(core::serializer &s, const asset_ref<T> &ref) {
		return s << ref.name();
	}

	template<typename T> inline core::deserializer &operator>>(core::deserializer &s, asset_ref<T> &ref) {
		std::string name;
		s >> name;
		ref.set(name);
		return s;
	}
} // namespace polar::support::asset
