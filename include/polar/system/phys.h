#pragma once

#include <polar/support/phys/detector/base.h>
#include <polar/system/base.h>
#include <unordered_map>

namespace polar {
namespace system {
	class phys : public base {
	  private:
		template <typename T> struct pair_hasher {
			std::size_t operator()(const std::pair<T, T> &pair) const {
				std::size_t seed = 0;
				if(pair.first < pair.second) {
					boost::hash_combine(seed, pair.first);
					boost::hash_combine(seed, pair.second);
				} else {
					boost::hash_combine(seed, pair.second);
					boost::hash_combine(seed, pair.first);
				}
				return seed;
			}
		};

		template <typename T> struct pair_comparator {
			bool operator()(const std::pair<T, T> &lhs,
			                const std::pair<T, T> &rhs) const {
				auto lhsMin = std::min(lhs.first, lhs.second);
				auto lhsMax = std::max(lhs.first, lhs.second);
				auto rhsMin = std::min(rhs.first, rhs.second);
				auto rhsMax = std::max(rhs.first, rhs.second);
				return lhsMin == rhsMin && lhsMax == rhsMax;
			}
		};

		using ti_t          = const std::type_info *;
		using detector_base = support::phys::detector::base;
		using resolver_t    = std::function<bool(std::shared_ptr<detector_base>,
		                                      std::shared_ptr<detector_base>)>;

		std::unordered_map<std::pair<ti_t, ti_t>, std::shared_ptr<resolver_t>,
		                   pair_hasher<ti_t>, pair_comparator<ti_t>>
		    resolvers;

	  protected:
		void init() override final;

	  public:
		static bool supported() { return true; }
		phys(core::polar *engine) : base(engine) {}

		template <typename T, typename U,
		          typename = typename std::enable_if<
		              std::is_base_of<detector_base, T>::value>::type,
		          typename = typename std::enable_if<
		              std::is_base_of<detector_base, U>::value>::type>
		void add(std::function<bool(std::shared_ptr<T>, std::shared_ptr<U>)>
		             resolver) {
			// reinterpret_pointer_cast isn't standard yet
			auto typedPtr = new decltype(resolver)(resolver);
			auto ptr      = reinterpret_cast<resolver_t *>(typedPtr);
			auto sp       = std::shared_ptr<resolver_t>(ptr);
			auto pair     = std::make_pair(&typeid(T), &typeid(U));
			resolvers.emplace(pair, sp);
		}
	};
}
}
