#pragma once

#include <boost/circular_buffer.hpp>
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
	private:
		using tag_id       = support::action::tag::id;
		using tag_ti       = support::action::tag::ti;
		using relation     = support::action::relation;
		using bimap        = support::action::bimap;
		using binding_t    = support::action::binding_t;
		using lifetime     = support::action::lifetime;
		using digital_data = support::action::digital_data;
		using analog_state = support::action::analog_state;
		using analog_data  = support::action::analog_data;
		using digital_map  = support::action::digital_map;
		using analog_map   = support::action::analog_map;

		const int fps = 50;
		const DeltaTicks timestep = DeltaTicks(ENGINE_TICKS_PER_SECOND / fps);
		DeltaTicks accumulator;

		struct frame_action {
			IDType objectID;
			std::type_index ti;
			bool value;

			frame_action(IDType objectID, std::type_index ti, bool value) : objectID(objectID), ti(ti), value(value) {}
		};

		struct frame {
			digital_map digitals;
			analog_map analogs;
			std::vector<frame_action> actions;
		};

		boost::circular_buffer<frame> framebuffer = boost::circular_buffer<frame>(100, frame{});
		size_t frame_offset = 0;

		std::array<bimap, size_t(lifetime::SIZE)> lt_bindings;
		bimap bindings;

		IDType nextID = 1;
	public:
		static bool supported() { return true; }
		action(core::polar *engine) : base(engine) {}

		inline auto get_framebuffer() const { return framebuffer; }
		inline auto get_frame_offset() const { return frame_offset; }
		inline auto & current_frame(size_t n = 0) {
			return framebuffer[framebuffer.size() - 1 - frame_offset - n];
		}

		inline void force_tick() { tick(); }

		void update(DeltaTicks &dt) override {
			accumulator += dt;
			if(accumulator.Seconds() > 1.0f) { accumulator.SetSeconds(1.0f); }

			while(accumulator >= timestep) {
				tick();
				accumulator -= timestep;
			}
		}

		void tick() {
			auto &cf = current_frame();

			// copy to avoid invalidation
			auto digitals = cf.digitals;
			for(auto &pair : digitals) {
				for(auto &state : pair.second.states) {
					trigger_digital<false>(state.first, pair.first,
					                       state.second ? lifetime::when
					                                    : lifetime::unless);
				}
			}

			for(auto &pair : cf.analogs) {
				for(auto &state : pair.second.states) {
					trigger_analog(state.first, pair.first);
				}
			}

			if(frame_offset == 0) {
				framebuffer.push_back(frame{cf.digitals, cf.analogs, {}});

				auto &nf = current_frame();

				// reset analog values immediately after triggering
				// this allows them to accumulate across the entire frame
				for(auto &pair : nf.analogs) {
					for(auto &state : pair.second.states) {
						state.second.previous = state.second.value;
						state.second.value = state.second.initial;
					}
				}
			} else {
				--frame_offset;

				auto &nf = current_frame();

				auto tmp = nf.digitals;
				nf.digitals = cf.digitals;

				for(auto &pair : tmp) {
					auto it = nf.digitals.find(pair.first);
					if(it == nf.digitals.end()) {
						nf.digitals.emplace(pair.first, pair.second);
					}
					//nf.digitals[pair.first] = pair.second;
				}

				for(auto &a : cf.actions) {
					trigger_digital<false>(a.objectID, a.ti, a.value);
				}

				auto tmp2 = nf.analogs;
				nf.analogs = cf.analogs;

				for(auto &pair : tmp2) {
					auto it = nf.analogs.find(pair.first);
					if(it == nf.analogs.end()) {
						nf.analogs.emplace(pair.first, pair.second);
					}
				}
			}
		}

		inline bool revert_by(size_t n = 1) {
			if(n == 0) { return true; }

			auto size = framebuffer.size();
			if(n >= size) {
				return false;
			} else {
				frame_offset = n;
				return true;
			}
		}

		void reg_digital(IDType objectID, std::type_index ti, bool state = false) {
			auto &cf = current_frame();

			auto it = cf.digitals.find(ti);
			if(it == cf.digitals.end()) {
				cf.digitals.emplace(ti, digital_data{});
			}
			cf.digitals[ti].states.emplace(objectID, state);
		}

		void reg_analog(IDType objectID, std::type_index ti, Decimal initial = 0) {
			auto &cf = current_frame();

			auto it = cf.analogs.find(ti);
			if(it == cf.analogs.end()) {
				cf.analogs.emplace(ti, analog_data{});
			}
			cf.analogs[ti].states.emplace(objectID, analog_state{initial, initial, initial});
		}

		template<typename T>
		typename std::enable_if<std::is_base_of<digital, T>::value>::type
		reg_digital(IDType objectID, bool state = false) { reg_digital(objectID, typeid(T), state); }

		template<typename T>
		typename std::enable_if<std::is_base_of<analog, T>::value>::type
		reg_analog(IDType objectID, Decimal initial = 0) { reg_analog(objectID, typeid(T), initial); }

		// digital -> digital function
		template<typename Src,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type>
		auto bind(lifetime lt, digital_function_t f, priority_t priority = 0, bool cont = true) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create<Src>(f);
			b.cont = cont;
			auto r  = relation{id, ti, priority, b};
			lt_bindings[size_t(lt)].insert(r);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].get<tag_id>().erase(id);
			});
		}

		// digital -> digital function with ID
		template<typename Src,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type>
		auto bind(IDType objectID, lifetime lt, digital_function_t f, priority_t priority = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create<Src>(f);
			b.objectID = objectID;
			auto r  = relation{id, ti, priority, b};
			lt_bindings[size_t(lt)].insert(r);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].get<tag_id>().erase(id);
			});
		}

		// digital -> digital
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Tgt>::value>::type>
		auto bind(lifetime lt, priority_t priority = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create_digital<Src, Tgt>();
			auto r  = relation{id, ti, priority, b};
			lt_bindings[size_t(lt)].insert(r);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].get<tag_id>().erase(id);
			});
		}

		// digital -> digital with ID
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Tgt>::value>::type>
		auto bind(IDType objectID, lifetime lt, priority_t priority = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create_digital<Src, Tgt>();
			b.objectID = objectID;
			auto r  = relation{id, ti, priority, b};
			lt_bindings[size_t(lt)].insert(r);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].get<tag_id>().erase(id);
			});
		}

		// digital -> analog
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Tgt>::value>::type>
		auto bind(lifetime lt, Decimal passthrough, priority_t priority = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create<Src, Tgt>(passthrough);
			auto r  = relation{id, ti, priority, b};
			lt_bindings[size_t(lt)].insert(r);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].get<tag_id>().erase(id);
			});
		}

		// digital -> analog with ID
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Tgt>::value>::type>
		auto bind(IDType objectID, lifetime lt, Decimal passthrough, priority_t priority = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create<Src, Tgt>(passthrough);
			b.objectID = objectID;
			auto r  = relation{id, ti, priority, b};
			lt_bindings[size_t(lt)].insert(r);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].get<tag_id>().erase(id);
			});
		}

		// analog -> analog function
		template<typename Src>
		auto bind(analog_function_t f, priority_t priority = 0,
		          typename std::enable_if<std::is_base_of<analog, Src>::value>::type* = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create<Src>(f);
			auto r  = relation{id, ti, priority, b};
			bindings.insert(r);
			return core::ref([this, id] {
				bindings.get<tag_id>().erase(id);
			});
		}

		// analog -> digital
		template<typename Src, typename Tgt>
		auto bind(analog_predicate_t p, priority_t priority = 0,
		          typename std::enable_if<std::is_base_of<analog, Src>::value>::type* = 0,
		          typename std::enable_if<std::is_base_of<digital, Tgt>::value>::type* = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create<Src, Tgt>(p);
			auto r  = relation{id, ti, priority, b};
			bindings.insert(r);
			return core::ref([this, id] {
				bindings.get<tag_id>().erase(id);
			});
		}

		// analog -> analog
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Tgt>::value>::type>
		auto bind(priority_t priority = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create_analog<Src, Tgt>();
			auto r = relation{id, ti, priority, b};
			bindings.insert(r);
			return core::ref([this, id] {
				bindings.get<tag_id>().erase(id);
			});
		}

		// analog -> analog with ID
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Tgt>::value>::type>
		auto bind(IDType objectID, priority_t priority = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create_analog<Src, Tgt>();
			b.objectID = objectID;
			auto r  = relation{id, ti, priority, b};
			bindings.insert(r);
			return core::ref([this, id] {
				bindings.get<tag_id>().erase(id);
			});
		}

		template<bool source = true>
		void trigger_digital(IDType objectID, std::type_index ti, lifetime lt) {
			debugmanager()->trace("triggering digital ", ti.name(), " for ", lt);

			auto sourceObjectID = objectID;

			if(lt == lifetime::on) {
				auto whenPair = lt_bindings[size_t(lifetime::when)].get<tag_ti>().equal_range(ti);
				for(auto whenIt = whenPair.first; whenIt != whenPair.second; ++whenIt) {
					if(whenIt->binding.objectID) { objectID = *whenIt->binding.objectID; }

					if(auto wrapper = whenIt->binding.get_if_tgt_digital()) {
						trigger_digital<false>(objectID, wrapper->ti, true);
					}

					if(!whenIt->binding.should_continue(objectID)) {
						break;
					}
				}
			} else if(lt == lifetime::after) {
				auto whenPair = lt_bindings[size_t(lifetime::when)].get<tag_ti>().equal_range(ti);
				for(auto whenIt = whenPair.first; whenIt != whenPair.second; ++whenIt) {
					if(whenIt->binding.objectID) { objectID = *whenIt->binding.objectID; }

					if(auto wrapper = whenIt->binding.get_if_tgt_digital()) {
						trigger_digital<false>(objectID, wrapper->ti, false);
					}

					if(!whenIt->binding.should_continue(objectID)) {
						break;
					}
				}
			}

			auto pair = lt_bindings[size_t(lt)].get<tag_ti>().equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
			//auto &view = lt_bindings[size_t(lt)].get<tag_ti>();
			//for(auto it = view.lower_bound(ti); it != view.end() && it->ti == ti; ++it) {
				if(it->binding.objectID) { objectID = *it->binding.objectID; }

				if(auto wrapper = it->binding.get_if_tgt_digital()) {
					if(lt != lifetime::when) {
						trigger_digital<false>(objectID, wrapper->ti);
					}
				} else if(auto wrapper = it->binding.get_if_tgt_analog()) {
					accumulate_analog(objectID, wrapper->ti, it->binding.passthrough.value_or(0));
				} else if(auto f = it->binding.get_if_tgt_digital_f()) {
					(*f)(objectID);
				}

				if(!it->binding.should_continue(objectID)) {
					break;
				}
			}

			if(source) {
				if(lt == lifetime::on) {
					framebuffer.back().actions.emplace_back(sourceObjectID, ti, true);
				} else if(lt == lifetime::after) {
					framebuffer.back().actions.emplace_back(sourceObjectID, ti, false);
				}
			}
		}

		template<bool source = true>
		void trigger_digital(IDType objectID, std::type_index ti, bool state) {
			auto &cf = current_frame();

			// force registration of digital
			reg_digital(objectID, ti);

			if(state != cf.digitals[ti].states[objectID]) {
				if(state) {
					trigger_digital<source>(objectID, ti, lifetime::on);
					cf.digitals[ti].states[objectID] = state;
				} else {
					cf.digitals[ti].states[objectID] = state;
					trigger_digital<source>(objectID, ti, lifetime::after);
				}
			}
		}

		template<bool source = true>
		void trigger_digital(IDType objectID, std::type_index ti) {
			trigger_digital<source>(objectID, ti, true);
			trigger_digital<source>(objectID, ti, lifetime::when);
			trigger_digital<source>(objectID, ti, false);
		}

		template<bool source = true>
		void trigger_analog(IDType objectID, std::type_index ti) {
			auto &cf = current_frame();

			// force registration of analog
			reg_analog(objectID, ti);

			auto &data = cf.analogs.at(ti);

			debugmanager()->trace("triggering analog ", ti.name(), '(', data.states[objectID].value, ')');

			auto pair = bindings.get<tag_ti>().equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
				if(auto f = it->binding.get_if_tgt_analog_f()) {
					(*f)(objectID, data.states[objectID].value);
				} else if(auto wrapper = it->binding.get_if_tgt_digital()) {
					auto result = it->binding.predicate(objectID, data.states[objectID].previous, data.states[objectID].value);
					trigger_digital<false>(objectID, wrapper->ti, result);
				}

				if(!it->binding.should_continue(objectID, data.states[objectID].value)) {
					break;
				}
			}
		}

		void accumulate_analog(IDType objectID, std::type_index ti, Decimal passthrough) {
			auto &cf = current_frame();

			// force registration of analog
			reg_analog(objectID, ti);

			cf.analogs[ti].states[objectID].value += passthrough;

			auto pair = bindings.get<tag_ti>().equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
				if(it->binding.objectID) { objectID = *it->binding.objectID; }

				if(auto wrapper = it->binding.get_if_tgt_analog()) {
					accumulate_analog(objectID, wrapper->ti, passthrough);
				}

				if(!it->binding.should_continue(objectID, passthrough)) {
					break;
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
