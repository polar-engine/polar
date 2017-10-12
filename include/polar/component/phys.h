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
		detector_t detector;
		responder_t responder;

		phys(detector_t d, responder_t r) : detector(d), responder(r) {}
	};
}
}
