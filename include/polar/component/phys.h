#pragma once

#include <polar/component/base.h>
#include <polar/support/phys/detector/base.h>
#include <polar/support/phys/responder/base.h>

namespace polar::component {
	class phys : public base {
		using detector_t  = support::phys::detector::base;
		using responder_t = support::phys::responder::base;

	  public:
		std::shared_ptr<detector_t> detector;
		std::vector<std::shared_ptr<responder_t>> responders;

		template<
			typename Det,
			typename = typename std::enable_if<std::is_base_of<detector_t, Det>::value>::type
		>
		phys(Det d) : detector(std::static_pointer_cast<detector_t>(std::make_shared<Det>(d))) {}

		template<
			typename Det, typename Res, typename... Ts,
			typename = typename std::enable_if<std::is_base_of<responder_t, Res>::value>::type
		>
		phys(Det d, Res r, Ts && ...args) : phys(d, std::forward<Ts>(args)...) {
			responders.emplace_back(
				std::static_pointer_cast<responder_t>(std::make_shared<Res>(r))
			);
		}

		virtual std::string name() const override { return "phys"; }
	};
} // namespace polar::component
