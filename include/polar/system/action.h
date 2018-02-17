#pragma once

#include <variant>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <polar/support/action/lifetime.h>
#include <polar/system/base.h>

namespace polar::system {
	class action : public system::base {
	public:
		struct digital {};
		struct analog {};

		using digital_function_t = std::function<void()>;
		using analog_function_t  = std::function<void(Decimal)>;

		struct digital_data {
			bool state = false;
		};

		struct analog_data {
			const Decimal initial = 0;
			Decimal value = 0;
		};

		using digital_map = std::unordered_map<std::type_index, digital_data>;
		using analog_map  = std::unordered_map<std::type_index, analog_data>;

		class binding {
		  private:
			struct digital_wrapper { std::type_index ti; };
			struct analog_wrapper  { std::type_index ti; };

			std::variant<digital_wrapper, analog_wrapper> source;
		  public:
			std::variant <digital_wrapper, analog_wrapper,
			              digital_function_t, analog_function_t> target;
			Decimal passthrough;

			binding(decltype(source) src, decltype(target) tgt, Decimal pt = 0)
				: source(src), target(tgt), passthrough(pt) {}

			// digital -> digital function
			template<typename Src,
			         typename = typename std::enable_if<
			             std::is_base_of<digital, Src>::value>::type>
			static binding create(digital_function_t f) {
				auto src = digital_wrapper{typeid(Src)};
				return binding(src, f);
			}

			// digital -> digital
			template<typename Src, typename Tgt,
			         typename = typename std::enable_if<
			             std::is_base_of<digital, Src>::value>::type,
			         typename = typename std::enable_if<
			             std::is_base_of<digital, Tgt>::value>::type>
			static binding create_digital() {
				auto src = digital_wrapper{typeid(Src)};
				auto tgt = digital_wrapper{typeid(Tgt)};
				return binding(src, tgt);
			}

			// digital -> analog
			template<typename Src, typename Tgt,
			         typename = typename std::enable_if<
			             std::is_base_of<digital, Src>::value>::type,
			         typename = typename std::enable_if<
			             std::is_base_of<analog, Tgt>::value>::type>
			static binding create(Decimal passthrough) {
				auto src = digital_wrapper{typeid(Src)};
				auto tgt = analog_wrapper{typeid(Tgt)};
				return binding(src, tgt, passthrough);
			}

			// analog -> analog function
			template<typename Src,
			         typename = typename std::enable_if<
			             std::is_base_of<analog, Src>::value>::type>
			static binding create(analog_function_t f) {
				auto src = analog_wrapper{typeid(Src)};
				return binding(src, f);
			}

			// analog -> analog
			template<typename Src, typename Tgt,
			         typename = typename std::enable_if<
			             std::is_base_of<analog, Src>::value>::type,
			         typename = typename std::enable_if<
			             std::is_base_of<analog, Tgt>::value>::type>
			static binding create_analog() {
				auto src = analog_wrapper{typeid(Src)};
				auto tgt = analog_wrapper{typeid(Tgt)};
				return binding(src, tgt);
			}

			auto get_if_src_digital() {
				return std::get_if<digital_wrapper>(&source);
			}

			auto get_if_src_analog() {
				return std::get_if<analog_wrapper>(&source);
			}

			auto get_if_tgt_digital() {
				return std::get_if<digital_wrapper>(&target);
			}

			auto get_if_tgt_analog() {
				return std::get_if<analog_wrapper>(&target);
			}

			auto get_if_tgt_digital_f() {
				return std::get_if<digital_function_t>(&target);
			}

			auto get_if_tgt_analog_f() {
				return std::get_if<analog_function_t>(&target);
			}
		};
		
		using binding_bimap =
		    boost::bimap<boost::bimaps::unordered_multiset_of<std::type_index>,
		                 boost::bimaps::unordered_set_of<IDType>,
		                 boost::bimaps::set_of_relation<>,
		                 boost::bimaps::with_info<binding>>;
	private:
		using lifetime = support::action::lifetime;

		digital_map digitals;
		analog_map analogs;

		std::array<binding_bimap, size_t(lifetime::SIZE)> lt_bindings;
		binding_bimap bindings;

		IDType nextID = 1;
	public:
		static bool supported() { return true; }
		action(core::polar *engine) : base(engine) {}

		void update(DeltaTicks &) override {
			for(auto &pair : digitals) {
				trigger_digital(pair.first,
				                pair.second.state ? lifetime::when
				                                  : lifetime::unless);
			}

			for(auto &pair : analogs) {
				trigger_analog(pair.first);

				/* reset value immediately after triggering
				 * this allows the value to accumulate across the entire frame
				 */
				pair.second.value = pair.second.initial;
			}
		}

		void reg_digital(std::type_index ti, bool state = false) {
			digitals.emplace(ti, digital_data{state});
		}

		void reg_analog(std::type_index ti, Decimal initial = 0) {
			analogs.emplace(ti, analog_data{initial, initial});
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
			auto v  = binding_bimap::value_type(ti, id, binding::create<Src>(f));
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
			auto v  = binding_bimap::value_type(ti, id, b);
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
			auto v  = binding_bimap::value_type(ti, id, binding::create<Src, Tgt>(passthrough));
			lt_bindings[size_t(lt)].insert(v);
			return core::ref([this, lt, id] {
				lt_bindings[size_t(lt)].right.erase(id);
			});
		}

		// analog -> analog function
		template<typename Src,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Src>::value>::type>
		auto bind(analog_function_t f) {
			std::type_index ti = typeid(Src);
			auto id = nextID++;
			auto v  = binding_bimap::value_type(ti, id, binding::create<Src>(f));
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
			auto v  = binding_bimap::value_type(ti, id, b);
			bindings.insert(v);
			return core::ref([this, id] {
				bindings.right.erase(id);
			});
		}

		void trigger_digital(std::type_index ti, lifetime lt) {
			debugmanager()->trace("triggering digital ", ti.name(), " for ", lt);

			auto pair = lt_bindings[size_t(lt)].left.equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
				auto &binding = it->info;
				if(auto wrapper = binding.get_if_tgt_digital()) {
					trigger_digital(wrapper->ti);
				} else if(auto wrapper = binding.get_if_tgt_analog()) {
					accumulate_analog(wrapper->ti, binding.passthrough);
				} else if(auto f = binding.get_if_tgt_digital_f()) {
					(*f)();
				}
			}
		}

		void trigger_digital(std::type_index ti, bool state) {
			// force registration of digital
			reg_digital(ti);

			if(state) {
				trigger_digital(ti, lifetime::on);
				digitals[ti].state = state;
			} else {
				digitals[ti].state = state;
				trigger_digital(ti, lifetime::after);
			}
		}

		void trigger_digital(std::type_index ti) {
			trigger_digital(ti, true);
			trigger_digital(ti, false);
		}

		void trigger_analog(std::type_index ti) {
			// force registration of analog
			reg_analog(ti);

			auto &data = analogs.at(ti);

			debugmanager()->trace("triggering analog ", ti.name(), '(', data.value, ')');

			auto pair = bindings.left.equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
				auto &binding = it->info;
				if(auto f = binding.get_if_tgt_analog_f()) {
					(*f)(data.value);
				}
			}
		}

		void accumulate_analog(std::type_index ti, Decimal passthrough) {
			// force registration of analog
			reg_analog(ti);

			analogs.at(ti).value += passthrough;

			auto pair = bindings.left.equal_range(ti);
			for(auto it = pair.first; it != pair.second; ++it) {
				auto &binding = it->info;
				if(auto wrapper = binding.get_if_tgt_analog()) {
					accumulate_analog(wrapper->ti, passthrough);
				}
			}
		}

		template<typename T,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, T>::value>::type>
		inline void trigger(lifetime lt) { trigger_digital(typeid(T), lt); }

		template<typename T,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, T>::value>::type>
		inline void trigger(bool state) { trigger_digital(typeid(T), state); }

		template<typename T>
		typename std::enable_if<std::is_base_of<digital, T>::value>::type
		trigger() { trigger_digital(typeid(T)); }

		template<typename T>
		typename std::enable_if<std::is_base_of<analog, T>::value>::type
		trigger() { trigger_analog(typeid(T)); }

		template<typename T,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, T>::value>::type>
		inline void accumulate(Decimal x) { accumulate_analog(typeid(T), x); }
	};
}
