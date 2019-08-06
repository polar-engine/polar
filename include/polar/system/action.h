#pragma once

#include <polar/support/action/binding.h>
#include <polar/support/action/lifetime.h>
#include <polar/system/base.h>

namespace polar::system {
	class action : public system::base {
	public:
		using digital            = support::action::digital;
		using analog             = support::action::analog;
		using digital_function_t = support::action::digital_function_t;
		using analog_function_t  = support::action::analog_function_t;
		using analog_predicate_t = support::action::analog_predicate_t;

		struct digital_data {
			std::unordered_map<IDType, bool> states;
		};

		struct analog_state {
			const Decimal initial = 0;
			Decimal previous = 0;
			Decimal value = 0;
		};

		struct analog_data {
			std::unordered_map<IDType, analog_state> states;
		};

		using digital_map = std::unordered_map<std::type_index, digital_data>;
		using analog_map  = std::unordered_map<std::type_index, analog_data>;
	private:
		using binding  = support::action::binding;
		using lifetime = support::action::lifetime;

		digital_map digitals;
		analog_map analogs;

		std::array<binding::bimap, size_t(lifetime::SIZE)> lt_bindings;
		binding::bimap bindings;

		IDType nextID = 1;

		const int fps = 60;
		const DeltaTicks timestep = DeltaTicks(ENGINE_TICKS_PER_SECOND / fps);
		DeltaTicks accumulator;
	public:
		static bool supported() { return true; }
		action(core::polar *engine) : base(engine) {}

		void update(DeltaTicks &dt) override {
			accumulator += dt;
			if(accumulator.Seconds() > 1.0f) { accumulator.SetSeconds(1.0f); }

			while(accumulator >= timestep) {
				tick();
				accumulator -= timestep;
			}
		}

		void tick() {
			for(auto &pair : digitals) {
				for(auto &state : pair.second.states) {
					trigger_digital(state.first, pair.first,
					                state.second ? lifetime::when
					                             : lifetime::unless);
				}
			}

			for(auto &pair : analogs) {
				for(auto &state : pair.second.states) {
					trigger_analog(state.first, pair.first);

					/* reset value immediately after triggering
					 * this allows the value to accumulate across the entire frame
					 */
					state.second.previous = state.second.value;
					state.second.value = state.second.initial;
				}
			}
		}

		void reg_digital(IDType objectID, std::type_index ti, bool state = false) {
			auto it = digitals.find(ti);
			if(it == digitals.end()) {
				digitals.emplace(ti, digital_data{});
			}
			digitals[ti].states.emplace(objectID, state);
		}

		void reg_analog(IDType objectID, std::type_index ti, Decimal initial = 0) {
			auto it = analogs.find(ti);
			if(it == analogs.end()) {
				analogs.emplace(ti, analog_data{});
			}
			analogs[ti].states.emplace(objectID, analog_state{initial, initial, initial});
		}

		template<typename T>
		typename std::enable_if<std::is_base_of<digital, T>::value>::type
		reg_digital(bool state = false) { reg_digital(typeid(T), state); }

		template<typename T>
		typename std::enable_if<std::is_base_of<analog, T>::value>::type
		reg_analog(Decimal initial = 0) { reg_analog(typeid(T), initial); }

		// digital -> digital function
		template<typename Src,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type>
		auto bind(lifetime lt, digital_function_t f) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b = binding::create<Src>(f);
			auto v  = binding::bimap::value_type(ti, id, b);
			lt_bindings[size_t(lt)].insert(v);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].right.erase(id);
			});
		}

		// digital -> digital function with ID
		template<typename Src,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type>
		auto bind(IDType objectID, lifetime lt, digital_function_t f) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b = binding::create<Src>(f);
			b.objectID = objectID;
			auto v  = binding::bimap::value_type(ti, id, b);
			lt_bindings[size_t(lt)].insert(v);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].right.erase(id);
			});
		}

		// digital -> digital
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Tgt>::value>::type>
		auto bind(lifetime lt) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b = binding::create_digital<Src, Tgt>();
			auto v  = binding::bimap::value_type(ti, id, b);
			lt_bindings[size_t(lt)].insert(v);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].right.erase(id);
			});
		}

		// digital -> digital with ID
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Tgt>::value>::type>
		auto bind(IDType objectID, lifetime lt) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b = binding::create_digital<Src, Tgt>();
			b.objectID = objectID;
			auto v  = binding::bimap::value_type(ti, id, b);
			lt_bindings[size_t(lt)].insert(v);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].right.erase(id);
			});
		}

		// digital -> analog
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Tgt>::value>::type>
		auto bind(lifetime lt, Decimal passthrough) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b = binding::create<Src, Tgt>(passthrough);
			auto v  = binding::bimap::value_type(ti, id, b);
			lt_bindings[size_t(lt)].insert(v);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].right.erase(id);
			});
		}

		// digital -> analog with ID
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Tgt>::value>::type>
		auto bind(IDType objectID, lifetime lt, Decimal passthrough) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b = binding::create<Src, Tgt>(passthrough);
			b.objectID = objectID;
			auto v  = binding::bimap::value_type(ti, id, b);
			lt_bindings[size_t(lt)].insert(v);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].right.erase(id);
			});
		}

		// analog -> analog function
		template<typename Src>
		auto bind(analog_function_t f,
		          typename std::enable_if<std::is_base_of<analog, Src>::value>::type* = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto v  = binding::bimap::value_type(ti, id, binding::create<Src>(f));
			bindings.insert(v);
			return core::ref([this, id] {
				bindings.right.erase(id);
			});
		}

		// analog -> digital
		template<typename Src, typename Tgt>
		auto bind(analog_predicate_t p,
		          typename std::enable_if<std::is_base_of<analog, Src>::value>::type* = 0,
		          typename std::enable_if<std::is_base_of<digital, Tgt>::value>::type* = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto v  = binding::bimap::value_type(ti, id, binding::create<Src, Tgt>(p));
			bindings.insert(v);
			return core::ref([this, id] {
				bindings.right.erase(id);
			});
		}

		// analog -> analog
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Tgt>::value>::type>
		auto bind() {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b = binding::create_analog<Src, Tgt>();
			auto v  = binding::bimap::value_type(ti, id, b);
			bindings.insert(v);
			return core::ref([this, id] {
				bindings.right.erase(id);
			});
		}

		// analog -> analog with ID
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Tgt>::value>::type>
		auto bind(IDType objectID) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b = binding::create_analog<Src, Tgt>();
			b.objectID = objectID;
			auto v  = binding::bimap::value_type(ti, id, b);
			bindings.insert(v);
			return core::ref([this, id] {
				bindings.right.erase(id);
			});
		}

		void trigger_digital(IDType objectID, std::type_index ti, lifetime lt) {
			debugmanager()->trace("triggering digital ", ti.name(), " for ", lt);

			if(lt == lifetime::when) {
				debugmanager()->trace("asdf");
			}

			if(lt == lifetime::on) {
				auto whenPair = lt_bindings[size_t(lifetime::when)].left.equal_range(ti);
				for(auto whenIt = whenPair.first; whenIt != whenPair.second; ++whenIt) {
					auto &binding = whenIt->info;
					if(binding.objectID) { objectID = *binding.objectID; }

					if(auto wrapper = binding.get_if_tgt_digital()) {
						trigger_digital(objectID, wrapper->ti, true);
					}
				}
			} else if(lt == lifetime::after) {
				auto whenPair = lt_bindings[size_t(lifetime::when)].left.equal_range(ti);
				for(auto whenIt = whenPair.first; whenIt != whenPair.second; ++whenIt) {
					auto &binding = whenIt->info;
					if(binding.objectID) { objectID = *binding.objectID; }

					if(auto wrapper = binding.get_if_tgt_digital()) {
						trigger_digital(objectID, wrapper->ti, false);
					}
				}
			}

			auto pair = lt_bindings[size_t(lt)].left.equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
				auto &binding = it->info;
				if(binding.objectID) { objectID = *binding.objectID; }

				if(auto wrapper = binding.get_if_tgt_digital()) {
					if(lt != lifetime::when) {
						trigger_digital(objectID, wrapper->ti);
					}
				} else if(auto wrapper = binding.get_if_tgt_analog()) {
					accumulate_analog(objectID, wrapper->ti, binding.passthrough);
				} else if(auto f = binding.get_if_tgt_digital_f()) {
					(*f)(objectID);
				}
			}
		}

		void trigger_digital(IDType objectID, std::type_index ti, bool state) {
			// force registration of digital
			reg_digital(objectID, ti);

			if(state != digitals[ti].states[objectID]) {
				if(state) {
					trigger_digital(objectID, ti, lifetime::on);
					digitals[ti].states[objectID] = state;
				} else {
					digitals[ti].states[objectID] = state;
					trigger_digital(objectID, ti, lifetime::after);
				}
			}
		}

		void trigger_digital(IDType objectID, std::type_index ti) {
			// XXX: TEMPORARY FIX, need to keep state continuously >:(
			trigger_digital(objectID, ti, true);
			trigger_digital(objectID, ti, lifetime::when);
			trigger_digital(objectID, ti, false);
		}

		void trigger_analog(IDType objectID, std::type_index ti) {
			// force registration of analog
			reg_analog(objectID, ti);

			auto &data = analogs.at(ti);

			debugmanager()->trace("triggering analog ", ti.name(), '(', data.states[objectID].value, ')');

			auto pair = bindings.left.equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
				auto &binding = it->info;
				if(auto f = binding.get_if_tgt_analog_f()) {
					(*f)(objectID, data.states[objectID].value);
				} else if(auto wrapper = binding.get_if_tgt_digital()) {
					auto result = binding.predicate(objectID, data.states[objectID].previous, data.states[objectID].value);
					trigger_digital(objectID, wrapper->ti, result);
				}
			}
		}

		void accumulate_analog(IDType objectID, std::type_index ti, Decimal passthrough) {
			// force registration of analog
			reg_analog(objectID, ti);

			analogs[ti].states[objectID].value += passthrough;

			auto pair = bindings.left.equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
				auto &binding = it->info;
				if(binding.objectID) { objectID = *binding.objectID; }

				if(auto wrapper = binding.get_if_tgt_analog()) {
					accumulate_analog(objectID, wrapper->ti, passthrough);
				}
			}
		}

		template<typename T,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, T>::value>::type>
		inline void trigger(IDType objectID, lifetime lt) { trigger_digital(objectID, typeid(T), lt); }

		template<typename T,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, T>::value>::type>
		inline void trigger(IDType objectID, bool state) { trigger_digital(objectID, typeid(T), state); }

		template<typename T>
		typename std::enable_if<std::is_base_of<digital, T>::value>::type
		trigger(IDType objectID) { trigger_digital(objectID, typeid(T)); }

		template<typename T>
		typename std::enable_if<std::is_base_of<analog, T>::value>::type
		trigger(IDType objectID) { trigger_analog(objectID, typeid(T)); }

		template<typename T,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, T>::value>::type>
		inline void accumulate(IDType objectID, Decimal x) { accumulate_analog(objectID, typeid(T), x); }
	};
}
