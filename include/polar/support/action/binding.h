#pragma once

#include <typeindex>
#include <variant>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <polar/support/action/types.h>

namespace polar::support::action {
	class binding {
	  public:
		using bimap =
		    boost::bimap<boost::bimaps::unordered_multiset_of<std::type_index>,
		                 boost::bimaps::unordered_set_of<IDType>,
		                 boost::bimaps::set_of_relation<>,
		                 boost::bimaps::with_info<binding>>;
	  private:
		struct digital_wrapper { std::type_index ti; };
		struct analog_wrapper  { std::type_index ti; };

		std::variant<digital_wrapper, analog_wrapper> source;
	  public:
		std::variant<digital_wrapper, analog_wrapper,
		             digital_function_t, analog_function_t> target;
		Decimal passthrough;
		analog_predicate_t predicate;
		std::optional<IDType> objectID;

		binding(decltype(source) src, decltype(target) tgt, Decimal pt = 0)
			: source(src), target(tgt), passthrough(pt) {}
		binding(decltype(source) src, decltype(target) tgt, decltype(predicate) p)
			: source(src), target(tgt), predicate(p) {}

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
		template<typename Src>
		static binding create(analog_function_t f,
		                      typename std::enable_if<std::is_base_of<analog, Src>::value>::type* = 0) {
			auto src = analog_wrapper{typeid(Src)};
			return binding(src, f);
		}

		// analog -> digital
		template<typename Src, typename Tgt>
		static binding create(analog_predicate_t p,
		                      typename std::enable_if<std::is_base_of<analog,  Src>::value>::type* = 0,
		                      typename std::enable_if<std::is_base_of<digital, Tgt>::value>::type* = 0) {
			auto src = analog_wrapper{typeid(Src)};
			auto tgt = digital_wrapper{typeid(Tgt)};
			return binding(src, tgt, p);
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
}
