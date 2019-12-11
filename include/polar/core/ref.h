#pragma once

#include <boost/container_hash/hash.hpp>
#include <polar/core/destructor.h>

namespace polar::core {
	class ref {
	public:
		using dtor_type = std::shared_ptr<destructor>;
	private:
		dtor_type _dtor;
	public:
		ref(bool alloc = false) {
			if(alloc) { _dtor = std::make_shared<destructor>(); }
		}
		ref(std::function<void()> fn) : _dtor(std::make_shared<destructor>(fn)) {}

		auto dtor() const {
			return _dtor;
		}

		friend inline bool operator==(const ref &lhs, const ref &rhs) {
			return lhs._dtor == rhs._dtor;
		}

		friend inline bool operator<(const ref &lhs, const ref &rhs) {
			return lhs._dtor < rhs._dtor;
		}
	};

	class weak_ref {
	public:
		using dtor_type = std::weak_ptr<destructor>;
	private:
		dtor_type _dtor;
	public:
		weak_ref() = default;
		weak_ref(ref r) : _dtor(r.dtor()) {}

		auto dtor() const {
			return _dtor;
		}

		friend inline bool operator==(const weak_ref &lhs, const weak_ref &rhs) {
			return lhs._dtor.lock() == rhs._dtor.lock();
		}

		friend inline bool operator<(const weak_ref &lhs, const weak_ref &rhs) {
			return lhs._dtor.lock() < rhs._dtor.lock();
		}
	};

	template<typename T> inline size_t hash_value(const ref &r) {
		boost::hash<T> hasher;
		return hasher(r.dtor());
	}

	template<typename T> inline size_t hash_value(const weak_ref &r) {
		boost::hash<T> hasher;
		return hasher(r.dtor());
	}
} // namespace polar::core

namespace std {
	template<> struct hash<polar::core::ref> {
		inline size_t operator()(const polar::core::ref &r) const {
			return std::hash<polar::core::ref::dtor_type>()(r.dtor());
		}
	};

	template<> struct hash<polar::core::weak_ref> {
		inline size_t operator()(const polar::core::weak_ref &r) const {
			return std::hash<polar::core::ref::dtor_type>()(r.dtor().lock());
		}
	};
} // namespace std
