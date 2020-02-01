#pragma once

#include <boost/circular_buffer.hpp>
#include <polar/support/action/binding.h>
#include <polar/support/action/lifetime.h>
#include <polar/support/sched/clock/integrator.h>
#include <polar/system/base.h>
#include <polar/system/sched.h>

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

		struct frame_action {
			core::ref object;
			std::type_index ti;
			bool value;

			frame_action(core::ref object, std::type_index ti, bool value) : object(object), ti(ti), value(value) {}
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

		core::id nextID = 1;
	public:
		static bool supported() { return true; }
		action(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "action"; }

		inline auto get_framebuffer() const { return framebuffer; }
		inline auto get_frame_offset() const { return frame_offset; }
		inline auto & current_frame(size_t n = 0) {
			return framebuffer[framebuffer.size() - 1 - frame_offset - n];
		}

		inline void force_tick() { tick(); }

		void init() override {
			auto sch = engine->get<sched>().lock();
			keep(sch->bind<support::sched::clock::integrator>([this] (auto) {
				tick();
			}));
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
					trigger_digital<false>(a.object, a.ti, a.value);
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

		void reg_digital(core::ref object, std::type_index ti, bool state = false) {
			auto &cf = current_frame();

			auto it = cf.digitals.find(ti);
			if(it == cf.digitals.end()) {
				cf.digitals.emplace(ti, digital_data{});
			}
			cf.digitals[ti].states.emplace(object, state);
		}

		void reg_analog(core::ref object, std::type_index ti, math::decimal initial = 0) {
			auto &cf = current_frame();

			auto it = cf.analogs.find(ti);
			if(it == cf.analogs.end()) {
				cf.analogs.emplace(ti, analog_data{});
			}
			cf.analogs[ti].states.emplace(object, analog_state{initial, initial, initial});
		}

		template<typename T>
		typename std::enable_if<std::is_base_of<digital, T>::value>::type
		reg_digital(core::ref object, bool state = false) { reg_digital(object, typeid(T), state); }

		template<typename T>
		typename std::enable_if<std::is_base_of<analog, T>::value>::type
		reg_analog(core::ref object, math::decimal initial = 0) { reg_analog(object, typeid(T), initial); }

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
		auto bind(core::ref object, lifetime lt, digital_function_t f, priority_t priority = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create<Src>(f);
			b.object = object;
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
		auto bind(core::ref object, lifetime lt, priority_t priority = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create_digital<Src, Tgt>();
			b.object = object;
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
		auto bind(lifetime lt, math::decimal passthrough, priority_t priority = 0) {
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
		auto bind(core::ref object, lifetime lt, math::decimal passthrough, priority_t priority = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create<Src, Tgt>(passthrough);
			b.object = object;
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
		auto bind(core::ref object, priority_t priority = 0) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto b  = binding_t::create_analog<Src, Tgt>();
			b.object = object;
			auto r  = relation{id, ti, priority, b};
			bindings.insert(r);
			return core::ref([this, id] {
				bindings.get<tag_id>().erase(id);
			});
		}

		template<bool source = true>
		void trigger_digital(core::ref object, std::type_index ti, lifetime lt) {
			log()->trace("action", "triggering digital ", ti.name(), " for ", lt);

			auto sourceObjectID = object;

			if(lt == lifetime::on) {
				auto whenPair = lt_bindings[size_t(lifetime::when)].get<tag_ti>().equal_range(ti);
				for(auto whenIt = whenPair.first; whenIt != whenPair.second; ++whenIt) {
					if(whenIt->binding.object) { object = *whenIt->binding.object; }

					if(auto wrapper = whenIt->binding.get_if_tgt_digital()) {
						trigger_digital<false>(object, wrapper->ti, true);
					}

					if(!whenIt->binding.should_continue(object)) {
						break;
					}
				}
			} else if(lt == lifetime::after) {
				auto whenPair = lt_bindings[size_t(lifetime::when)].get<tag_ti>().equal_range(ti);
				for(auto whenIt = whenPair.first; whenIt != whenPair.second; ++whenIt) {
					if(whenIt->binding.object) { object = *whenIt->binding.object; }

					if(auto wrapper = whenIt->binding.get_if_tgt_digital()) {
						trigger_digital<false>(object, wrapper->ti, false);
					}

					if(!whenIt->binding.should_continue(object)) {
						break;
					}
				}
			}

			auto pair = lt_bindings[size_t(lt)].get<tag_ti>().equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
			//auto &view = lt_bindings[size_t(lt)].get<tag_ti>();
			//for(auto it = view.lower_bound(ti); it != view.end() && it->ti == ti; ++it) {
				if(it->binding.object) { object = *it->binding.object; }

				if(auto wrapper = it->binding.get_if_tgt_digital()) {
					if(lt != lifetime::when) {
						trigger_digital<false>(object, wrapper->ti);
					}
				} else if(auto wrapper = it->binding.get_if_tgt_analog()) {
					accumulate_analog(object, wrapper->ti, it->binding.passthrough.value_or(0));
				} else if(auto f = it->binding.get_if_tgt_digital_f()) {
					(*f)(object);
				}

				if(!it->binding.should_continue(object)) {
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
		void trigger_digital(core::ref object, std::type_index ti, bool state) {
			auto &cf = current_frame();

			// force registration of digital
			reg_digital(object, ti);

			if(state != cf.digitals[ti].states[object]) {
				if(state) {
					trigger_digital<source>(object, ti, lifetime::on);
					cf.digitals[ti].states[object] = state;
				} else {
					cf.digitals[ti].states[object] = state;
					trigger_digital<source>(object, ti, lifetime::after);
				}
			}
		}

		template<bool source = true>
		void trigger_digital(std::type_index ti, bool state) {
			return trigger_digital<source>(core::ref(), ti, state);
		}

		template<bool source = true>
		void trigger_digital(core::ref object, std::type_index ti) {
			trigger_digital<source>(object, ti, true);
			trigger_digital<source>(object, ti, lifetime::when);
			trigger_digital<source>(object, ti, false);
		}

		template<bool source = true>
		void trigger_analog(core::ref object, std::type_index ti) {
			auto &cf = current_frame();

			// force registration of analog
			reg_analog(object, ti);

			auto &data = cf.analogs.at(ti);

			log()->trace("action", "triggering analog ", ti.name(), '(', data.states[object].value, ')');

			auto pair = bindings.get<tag_ti>().equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
				if(auto f = it->binding.get_if_tgt_analog_f()) {
					(*f)(object, data.states[object].value);
				} else if(auto wrapper = it->binding.get_if_tgt_digital()) {
					auto result = it->binding.predicate(object, data.states[object].previous, data.states[object].value);
					trigger_digital<false>(object, wrapper->ti, result);
				}

				if(!it->binding.should_continue(object, data.states[object].value)) {
					break;
				}
			}
		}

		void accumulate_analog(std::type_index ti, math::decimal passthrough) {
			accumulate_analog(core::ref(), ti, passthrough);
		}

		void accumulate_analog(core::ref object, std::type_index ti, math::decimal passthrough) {
			auto &cf = current_frame();

			// force registration of analog
			reg_analog(object, ti);

			cf.analogs[ti].states[object].value += passthrough;

			auto pair = bindings.get<tag_ti>().equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
				if(it->binding.object) { object = *it->binding.object; }

				if(auto wrapper = it->binding.get_if_tgt_analog()) {
					accumulate_analog(object, wrapper->ti, passthrough);
				}

				if(!it->binding.should_continue(object, passthrough)) {
					break;
				}
			}
		}

		template<
			typename T,
			typename = typename std::enable_if<std::is_base_of<digital, T>::value>::type
		>
		inline void trigger(core::ref object, lifetime lt) { trigger_digital(object, typeid(T), lt); }

		template<
			typename T,
			typename = typename std::enable_if<std::is_base_of<digital, T>::value>::type
		>
		inline void trigger(core::ref object, bool state) { trigger_digital(object, typeid(T), state); }

		template<
			typename T,
			typename = typename std::enable_if<std::is_base_of<digital, T>::value>::type
		>
		inline void trigger(bool state) { trigger_digital(core::ref(), typeid(T), state); }

		template<typename T>
		typename std::enable_if<std::is_base_of<digital, T>::value>::type
		trigger(core::ref object) { trigger_digital(object, typeid(T)); }

		template<typename T>
		typename std::enable_if<std::is_base_of<digital, T>::value>::type
		trigger() { trigger_digital(core::ref(), typeid(T)); }

		template<typename T>
		typename std::enable_if<std::is_base_of<analog, T>::value>::type
		trigger(core::ref object) { trigger_analog(object, typeid(T)); }

		template<
			typename T,
			typename = typename std::enable_if<std::is_base_of<analog, T>::value>::type
		>
		inline void accumulate(core::ref object, math::decimal x) { accumulate_analog(object, typeid(T), x); }

		template<
			typename T,
			typename = typename std::enable_if<std::is_base_of<analog, T>::value>::type
		>
		inline void accumulate(math::decimal x) { accumulate_analog(core::ref(), typeid(T), x); }
	};
}
