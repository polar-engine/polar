#pragma once

#include <polar/component/base.h>
#include <polar/support/phys/detector/base.h>
#include <polar/support/phys/responder/base.h>

namespace polar {
namespace component {
	class phys : public base {
		using detector_t  = support::phys::detector::base;
		using responder_t = support::phys::responder::base;

	  public:
		std::shared_ptr<detector_t> detector;
		std::shared_ptr<responder_t> responder;

		template <typename Det, typename Res,
		          typename = typename std::enable_if<
		              std::is_base_of<detector_t, Det>::value>::type,
		          typename = typename std::enable_if<
		              std::is_base_of<responder_t, Res>::value>::type>
		phys(Det d, Res r)
		    : detector(std::static_pointer_cast<detector_t>(
		          std::make_shared<Det>(d))),
		      responder(std::static_pointer_cast<responder_t>(
		          std::make_shared<Res>(r))) {}
	};
}
}
