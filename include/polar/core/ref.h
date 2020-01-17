#pragma once

#include <boost/container_hash/hash.hpp>
#include <polar/core/destructor.h>
#include <polar/core/id.h>

namespace polar::core {
	class ref {
	public:
		using dtor_type = std::shared_ptr<destructor>;
	private:
		static core::id global_id;

		core::id _id;
		dtor_type _dtor;
	public:
		ref() = default;
		ref(std::function<void()> fn) : ref(std::make_shared<destructor>(fn)) {}
		ref(dtor_type dtor) : _id(global_id++), _dtor(dtor) {}

		auto dtor() const {
			return _dtor;
		}

		auto id() const {
			return _id;
		}

		operator core::id() const {
			return id();
		}

		friend inline bool operator==(const ref &lhs, const ref &rhs) {
			return lhs.id() == rhs.id();
		}

		friend inline bool operator<(const ref &lhs, const ref &rhs) {
			return lhs.id() < rhs.id();
		}
	};

	using weak_ref = core::id;
} // namespace polar::core

namespace std {
	template<> struct hash<polar::core::ref> {
		inline size_t operator()(const polar::core::ref &r) const {
			return std::hash<polar::core::id>()(r.id());
		}
	};
} // namespace std
