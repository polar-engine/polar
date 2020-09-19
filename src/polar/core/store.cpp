#include <polar/component/base.h>
#include <polar/core/polar.h>
#include <polar/core/store.h>
#include <sstream>

namespace polar::core {
	void store::serialize(polar *engine, std::ostream &os) {
		std::ostringstream ss_data;
		store_serializer ser(objects, workload, ss_data);

		std::vector<std::string> serialized_objects;

		// we cannot use iterators as workload may be invalidated
		for(size_t i = 0; i < workload.size(); ++i) {
			std::ostringstream ss_object;
			store_serializer ser_object(objects, workload, ss_object);

			auto &r = workload[i];
			auto comp_range = engine->objects.get<index::ref>().equal_range(r);

			std::vector<std::string> serialized_comps;

			for(auto comp_it = comp_range.first; comp_it != comp_range.second; ++comp_it) {
				std::ostringstream ss_comp;
				store_serializer ser_comp(objects, workload, ss_comp);

				auto &ptr = comp_it->ptr;
				ser_comp << ptr->name();
				if(ptr->serialize(ser_comp)) {
					serialized_comps.emplace_back(ss_comp.str());
				}
			}

			ser_object << serialized_comps;
			serialized_objects.emplace_back(ss_object.str());
		}

		store_serializer ser_final(objects, workload, os);
		ser_final << serialized_objects;
	}

	store store::deserialize(polar *engine, std::istream &is) {
		store st;

		store_deserializer deser(st.workload, is);

		std::vector<std::string> serialized_objects;
		deser >> serialized_objects;

		for(size_t i = 0; i < serialized_objects.size(); ++i) {
			st.insert(engine->add());
		}

		log()->trace("store", "object count = ", serialized_objects.size());

		for(size_t i = 0; i < serialized_objects.size(); ++i) {
			auto &serialized_object = serialized_objects[i];

			std::istringstream ss_object(serialized_object);
			store_deserializer deser_object(st.workload, ss_object);

			std::vector<std::string> serialized_comps;
			deser_object >> serialized_comps;

			log()->trace("store", "component count = ", serialized_comps.size());

			for(auto &serialized_comp : serialized_comps) {
				std::istringstream ss_comp(serialized_comp);
				store_deserializer deser_comp(st.workload, ss_comp);

				if(auto comp = registry::component::deserialize(deser_comp)) {
					engine->insert(st.workload[i], comp->ptr, comp->ti);
				}
			}
		}

		return st;
	}

	store_serializer &operator<<(store_serializer &s, const core::ref &r) {
		std::uint32_t index;

		auto it = s.objects.find(r);
		if(it != s.objects.end()) {
			index = it->second;
		} else {
			size_t i = s.workload.size();
			s.objects.emplace(r, i);
			s.workload.emplace_back(r);
			index = i;
		}

		return s << index;
	}

	store_deserializer &operator>>(store_deserializer &s, core::ref &r) {
		std::uint32_t index;
		s >> index;

		r = s.workload.at(index);

		return s;
	}
}
