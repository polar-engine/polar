#pragma once

#include <boost/container_hash/hash.hpp>
#include <polar/core/destructor.h>
#include <polar/core/id.h>

namespace polar::core {
	class weak_ref;

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
		ref(core::id id, dtor_type dtor) : _id(id), _dtor(dtor) {}

		inline auto id() const {
			return _id;
		}

		inline auto dtor() const {
			return _dtor;
		}

		inline operator core::id() const {
			return id();
		}

		friend inline bool operator==(const ref &lhs, const ref &rhs) {
			return lhs.id() == rhs.id();
		}

		friend inline bool operator<(const ref &lhs, const ref &rhs) {
			return lhs.id() < rhs.id();
		}
	};

	class weak_ref {
	  private:
		core::id _id;
		std::weak_ptr<destructor> _dtor;

	  public:
		weak_ref() = default;
		weak_ref(ref r) : _id(r.id()), _dtor(r.dtor()) {}

		inline auto id() const {
			return _id;
		}

		inline auto dtor() const {
			return _dtor;
		}

		inline auto own() const {
			return ref(id(), dtor().lock());
		}

		inline operator core::id() const {
			return id();
		}

		friend inline size_t hash_value(const weak_ref &wr) {
			size_t seed = 0;
			boost::hash_combine(seed, wr.id());
			return seed;
		}

		friend inline bool operator==(const weak_ref &lhs, const weak_ref &rhs) {
			return lhs.id() == rhs.id();
		}

		friend inline bool operator<(const weak_ref &lhs, const weak_ref &rhs) {
			return lhs.id() < rhs.id();
		}
	};

	//using weak_ref = core::id;
} // namespace polar::core

namespace std {
	template<> struct hash<polar::core::ref> {
		inline size_t operator()(const polar::core::ref &r) const {
			return std::hash<polar::core::id>()(r.id());
		}
	};

	template<> struct hash<polar::core::weak_ref> {
		inline size_t operator()(const polar::core::weak_ref &r) const {
			return std::hash<polar::core::id>()(r.id());
		}
	};
} // namespace std
