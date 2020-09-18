#include <polar/component/base.h>
#include <polar/core/polar.h>
#include <polar/core/store.h>
#include <sstream>

namespace polar::core {
	void store::serialize(polar *engine, std::ostream &os) {
		std::ostringstream ss_data;
		store_serializer ser(objects, workload, ss_data);

		// we cannot use iterators as workload may be invalidated
		for(size_t i = 0; i < workload.size(); ++i) {
			auto &r = workload[i];
			auto comp_range = engine->objects.get<index::ref>().equal_range(r);

			std::vector<std::string> comps;

			for(auto comp_it = comp_range.first; comp_it != comp_range.second; ++comp_it) {
				std::ostringstream ss_comp;
				store_serializer ser_comp(objects, workload, ss_comp);

				auto &ptr = comp_it->ptr;
				ser_comp << ptr->name();
				if(ptr->serialize(ser_comp)) {
					comps.emplace_back(ss_comp.str());
				}
			}

			ser << comps.size();
			for(auto &comp : comps) {
				ss_data << comp;
			}
		}

		store_serializer ser_final(objects, workload, os);
		ser_final << static_cast<const std::uint32_t>(workload.size());
		os << ss_data.str();
	}

	store_serializer &operator<<(store_serializer &s, const core::ref &r) {
		auto it = s.objects.find(r);
		if(it != s.objects.end()) {
			return s << it->second;
		} else {
			size_t i = s.workload.size();
			s.objects.emplace(r, i);
			s.workload.emplace_back(r);
			return s << i;
		}
	}
}
