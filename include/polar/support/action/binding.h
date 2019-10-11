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

		using source_type = std::variant<digital_wrapper, analog_wrapper>;
		using target_type = std::variant<digital_wrapper, analog_wrapper, digital_function_t, analog_function_t>;
		using cont_type   = std::variant<bool, digital_cont_t, analog_cont_t>;
		using pred_type   = analog_predicate_t;

		source_type source;

		binding_t(source_type src, target_type tgt) : source(src), target(tgt) {}
		binding_t(source_type src, target_type tgt, Decimal pt) : source(src), target(tgt), passthrough(pt) {}
		binding_t(source_type src, target_type tgt, pred_type p) : source(src), target(tgt), predicate(p) {}
	  public:
		target_type target;
		cont_type cont;
		pred_type predicate;
		std::optional<Decimal> passthrough;
		std::optional<IDType> objectID;

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

		auto get_cont_if_bool() const {
			return std::get_if<bool>(&cont);
		}

		auto get_cont_if_digital() const {
			return std::get_if<digital_cont_t>(&cont);
		}

		auto get_cont_if_analog() const {
			return std::get_if<analog_cont_t>(&cont);
		}

		bool should_continue(IDType sourceID) const {
			sourceID = objectID.value_or(sourceID);

			if(auto b = get_cont_if_bool()) {
				return *b;
			} else if(auto f = get_cont_if_digital()) {
				return (*f)(sourceID);
			}

			return true;
		}

		bool should_continue(IDType sourceID, Decimal value) const {
			value = passthrough.value_or(value);

			if(auto b = get_cont_if_bool()) {
				return *b;
			} else if(auto f = get_cont_if_analog()) {
				return (*f)(sourceID, value);
			}

			return true;
		}
	};

	namespace tag {
		struct id {};
		struct ti {};
	} // namespace tag

	struct relation {
		struct ti_comp {
			inline bool operator()(const relation &lhs, const relation &rhs) const {
				if(lhs.ti != rhs.ti) {
					return lhs.ti < rhs.ti;
				} else {
					return lhs.priority < rhs.priority;
				}
			}

			inline bool operator()(const relation &lhs, const std::type_index &rhs) const {
				return lhs.ti < rhs;
			}

			inline bool operator()(const std::type_index &lhs, const relation &rhs) const {
				return lhs < rhs.ti;
			}
		};

		IDType id;
		std::type_index ti;
		priority_t priority;
		binding_t binding;
	};

	using bimap = boost::multi_index_container<
		relation,
		boost::multi_index::indexed_by<
			boost::multi_index::hashed_unique     <boost::multi_index::tag<tag::id>, boost::multi_index::member<relation, IDType, &relation::id>>,
			boost::multi_index::ordered_non_unique<boost::multi_index::tag<tag::ti>, boost::multi_index::identity<relation>, relation::ti_comp>
		>
	>;
} // namespace polar::support::action
