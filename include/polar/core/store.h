#pragma once

#include <polar/core/ref.h>

namespace polar::core {
	class polar;

	class store {
	  private:
		std::map<ref, size_t> objects;
		std::vector<ref> workload;

	  public:
		inline void insert(ref r) {
			size_t i = workload.size();
			objects.emplace(r, i);
			workload.emplace_back(r);
		}

		inline void insert(weak_ref wr) {
			insert(wr.own());
		}

		void serialize(polar *engine, std::ostream &os);
	};
} // namespace polar::core
