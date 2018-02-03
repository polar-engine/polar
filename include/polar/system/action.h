#pragma once

#include <variant>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <polar/support/action/lifetime.h>
#include <polar/system/base.h>

namespace polar::system {
	class action : public system::base {
	private:
		using lifetime = support::action::lifetime;
	public:
		template<typename T>
		struct ref : public core::ref {
			const IDType id;

			ref(std::function<void()> fn, const IDType id) : core::ref(fn), id(id) {}

			inline bool operator==(const ref<T> &rhs) const { return id == rhs.id; }
			inline bool operator< (const ref<T> &rhs) const { return id <  rhs.id; }
		};

		struct ref_hasher {
			template<typename T>
			inline std::size_t operator()(const ref<T> &r) const {
				return boost::hash<std::size_t>()(r.id);
			}
		};

		struct digital_tag {};
		struct analog_tag {};

		using digital_ref = ref<digital_tag>;
		using analog_ref  = ref<analog_tag>;

		using digital_function_t = std::function<void()>;
		using analog_function_t  = std::function<void(Decimal)>;

		struct digital_data {
			bool state = false;
		};

		struct analog_data {
			const Decimal initial = 0;
			Decimal value = 0;
		};

		using digital_map = std::unordered_map<IDType, digital_data>;
		using analog_map  = std::unordered_map<IDType, analog_data>;

		struct binding_t {
			std::variant<digital_ref, analog_ref> source;
			std::variant<digital_ref, analog_ref, digital_function_t, analog_function_t> target;
			Decimal passthrough;
		};
		
		using binding_bimap =
		    boost::bimap<boost::bimaps::unordered_multiset_of<IDType>,
		                 boost::bimaps::unordered_set_of<IDType>,
		                 boost::bimaps::set_of_relation<>,
		                 boost::bimaps::with_info<binding_t>>;
	private:
		digital_map digitals;
		analog_map analogs;

		std::array<binding_bimap, size_t(lifetime::SIZE)> lt_bindings;
		binding_bimap bindings;

		IDType nextID = 1;
	public:
		static bool supported() { return true; }
		action(core::polar *engine) : base(engine) {}

		void update(DeltaTicks &) override {
			for(auto &pair : analogs) {
				pair.second.value = pair.second.initial;
			}

			for(auto &pair : digitals) {
				trigger(pair.second.state ? lifetime::when
				                          : lifetime::unless,
				        pair.first);
			}

			for(auto &pair : analogs) {
				trigger(pair.first);
			}
		}

		digital_ref digital(bool state = false) {
			auto id = nextID++;
			digitals.emplace(id, digital_data{state});
			return digital_ref([this, id] { digitals.erase(id); }, id);
		}

		analog_ref analog(Decimal initial = 0) {
			auto id = nextID++;
			analogs.emplace(id, analog_data{initial, initial});
			return analog_ref([this, id] { analogs.erase(id); }, id);
		}

		core::ref bind(lifetime lt, digital_ref src, const digital_function_t f) {
			auto id = nextID++;
			lt_bindings[size_t(lt)].insert(binding_bimap::value_type(src.id, id, binding_t{src, f}));
			return core::ref([this, lt, id] { lt_bindings[size_t(lt)].right.erase(id); });
		}

		core::ref bind(lifetime lt, analog_ref src, const digital_function_t f) {
			auto id = nextID++;
			lt_bindings[size_t(lt)].insert(binding_bimap::value_type(src.id, id, binding_t{src, f}));
			return core::ref([this, lt, id] { lt_bindings[size_t(lt)].right.erase(id); });
		}

		core::ref bind(lifetime lt, digital_ref src, digital_ref tgt) {
			auto id = nextID++;
			lt_bindings[size_t(lt)].insert(binding_bimap::value_type(src.id, id, binding_t{src, tgt}));
			return core::ref([this, lt, id] { lt_bindings[size_t(lt)].right.erase(id); });
		}

		core::ref bind(lifetime lt, digital_ref src, analog_ref tgt, Decimal passthrough) {
			auto id = nextID++;
			lt_bindings[size_t(lt)].insert(binding_bimap::value_type(src.id, id, binding_t{src, tgt, passthrough}));
			return core::ref([this, lt, id] { lt_bindings[size_t(lt)].right.erase(id); });
		}

		core::ref bind(analog_ref src, const analog_function_t f) {
			auto id = nextID++;
			bindings.insert(binding_bimap::value_type(src.id, id, binding_t{src, f}));
			return core::ref([this, id] { bindings.right.erase(id); });
		}

		void trigger(lifetime lt, IDType id) {
			debugmanager()->trace("triggering digital ", id, " for ", lt);

			auto pair = lt_bindings[size_t(lt)].left.equal_range(id);
			for(auto it = pair.first; it != pair.second; ++it) {
				auto &binding = it->info;
				if(auto r = std::get_if<digital_ref>(&binding.target)) {
					trigger(*r);
				} else if(auto r = std::get_if<analog_ref>(&binding.target)) {
					accumulate(*r, binding.passthrough);
				} else if(auto f = std::get_if<digital_function_t>(&binding.target)) {
					(*f)();
				}
			}
		}

		void trigger(lifetime lt, digital_ref r) {
			trigger(lt, r.id);
		}

		void trigger(digital_ref r, bool state) {
			if(state) {
				trigger(lifetime::on, r);
				digitals[r.id].state = state;
			} else {
				digitals[r.id].state = state;
				trigger(lifetime::after, r);
			}
		}

		void trigger(digital_ref r) {
			trigger(r, true);
			trigger(r, false);
		}

		void trigger(IDType id) {
			auto &data = analogs.at(id);

			debugmanager()->trace("triggering analog ", id, '(', data.value, ')');

			auto pair = bindings.left.equal_range(id);
			for(auto it = pair.first; it != pair.second; ++it) {
				auto &binding = it->info;
				if(auto f = std::get_if<analog_function_t>(&binding.target)) {
					(*f)(data.value);
				}
			}
		}

		void accumulate(IDType id, Decimal passthrough) {
			analogs.at(id).value += passthrough;
		}

		void accumulate(analog_ref r, Decimal passthrough) {
			accumulate(r.id, passthrough);
		}
	};
}
