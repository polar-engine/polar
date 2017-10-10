#if !defined(POLAR_H)
#include <polar/core/polar.h>
#else
#pragma once

#include <memory>
#include <polar/component/base.h>
#include <polar/core/deltaticks.h>
#include <polar/core/destructor.h>
#include <vector>

namespace polar {
namespace system {
	class base {
	  protected:
		std::vector<std::shared_ptr<core::destructor>> dtors;
		core::polar *engine;

	  public:
		static bool supported() { return false; }

		base(core::polar *engine) : engine(engine) {}
		virtual ~base() {}

		virtual void init() {}
		virtual void update(DeltaTicks &) {}
		virtual void componentadded(IDType, const std::type_info *,
		                            std::weak_ptr<component::base>) {}
		virtual void componentremoved(IDType, const std::type_info *) {}
	};
}
}

#endif
