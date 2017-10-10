#pragma once

#include <functional>

template <typename T>
struct sharedptr_less : public std::binary_function<std::shared_ptr<T>,
                                                    std::shared_ptr<T>, bool> {
	inline bool operator()(const std::shared_ptr<T> &left,
	                       const std::shared_ptr<T> &right) const {
		return *left < *right;
	}
};
