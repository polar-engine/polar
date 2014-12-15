#pragma once

#include <queue>
#include "Atomic.h"

template<typename T> class PoolAllocator {
public:
	typedef T value_type;
	typedef std::deque<T *> pool_type;
	typedef std::size_t size_type;
private:
	Atomic<pool_type> pool;
public:
	size_type max_size() { return 1; }

	T * allocate(size_type n) {
		return pool.With<T *>([n] (pool_type &pool) {
			if(pool.empty()) {
				return reinterpret_cast<T *>(malloc(sizeof(T) * n));
			} else {
				T *p = pool.back();
				pool.pop_back();
				return p;
			}
		});
	}

	void deallocate(T *p, size_type n) {
		pool.With([p] (pool_type &pool) { pool.push_back(p); });
	}
};
