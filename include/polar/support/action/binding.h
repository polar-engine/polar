#pragma once

#include <typeindex>
#include <variant>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <polar/support/action/types.h>
#include <polar/util/prioritized.h>

namespace polar::support::action {
	class binding_t {
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

		binding_t(decltype(source) src, decltype(target) tgt, Decimal pt = 0)
			: source(src), target(tgt), passthrough(pt) {}
		binding_t(decltype(source) src, decltype(target) tgt, decltype(predicate) p)
			: source(src), target(tgt), predicate(p) {}

		// digital -> digital function
		template<typename Src,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type>
		static binding_t create(digital_function_t f) {
			auto src = digital_wrapper{typeid(Src)};
			return binding_t(src, f);
		}

		// digital -> digital
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Tgt>::value>::type>
		static binding_t create_digital() {
			auto src = digital_wrapper{typeid(Src)};
			auto tgt = digital_wrapper{typeid(Tgt)};
			return binding_t(src, tgt);
		}

		// digital -> analog
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<digital, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Tgt>::value>::type>
		static binding_t create(Decimal passthrough) {
			auto src = digital_wrapper{typeid(Src)};
			auto tgt = analog_wrapper{typeid(Tgt)};
			return binding_t(src, tgt, passthrough);
		}

		// analog -> analog function
		template<typename Src>
		static binding_t create(analog_function_t f,
		                      typename std::enable_if<std::is_base_of<analog, Src>::value>::type* = 0) {
			auto src = analog_wrapper{typeid(Src)};
			return binding_t(src, f);
		}

		// analog -> digital
		template<typename Src, typename Tgt>
		static binding_t create(analog_predicate_t p,
		                      typename std::enable_if<std::is_base_of<analog,  Src>::value>::type* = 0,
		                      typename std::enable_if<std::is_base_of<digital, Tgt>::value>::type* = 0) {
			auto src = analog_wrapper{typeid(Src)};
			auto tgt = digital_wrapper{typeid(Tgt)};
			return binding_t(src, tgt, p);
		}

		// analog -> analog
		template<typename Src, typename Tgt,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Src>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<analog, Tgt>::value>::type>
		static binding_t create_analog() {
			auto src = analog_wrapper{typeid(Src)};
			auto tgt = analog_wrapper{typeid(Tgt)};
			return binding_t(src, tgt);
		}

		auto get_if_src_digital() const {
			return std::get_if<digital_wrapper>(&source);
		}

		auto get_if_src_analog() const {
			return std::get_if<analog_wrapper>(&source);
		}

		auto get_if_tgt_digital() const {
			return std::get_if<digital_wrapper>(&target);
		}

		auto get_if_tgt_analog() const {
			return std::get_if<analog_wrapper>(&target);
		}

		auto get_if_tgt_digital_f() const {
			return std::get_if<digital_function_t>(&target);
		}

		auto get_if_tgt_analog_f() const {
			return std::get_if<analog_function_t>(&target);
		}
	};

	namespace tag {
		struct id {};
		struct ti {};
	} // namespace tag

	struct relation {
		IDType id;
		std::type_index ti;
		priority_t priority;
		binding_t binding;
	};

	using bimap = boost::multi_index_container<
		relation,
		boost::multi_index::indexed_by<
			boost::multi_index::hashed_unique    <boost::multi_index::tag<tag::id>, boost::multi_index::member<relation, IDType,          &relation::id>>,
			boost::multi_index::hashed_non_unique<boost::multi_index::tag<tag::ti>, boost::multi_index::member<relation, std::type_index, &relation::ti>>
		>
	>;
} // namespace polar::support::action
