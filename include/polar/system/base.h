#if !defined(POLAR_H)
#include <polar/core/polar.h>
#else
#pragma once

#include <memory>
#include <polar/component/base.h>
#include <polar/core/deltaticks.h>
#include <polar/core/destructor.h>
#include <vector>

namespace polar::system {
	class base {
	  private:
		std::vector<core::ref> dtors;
	  protected:
		core::polar *engine;

	  public:
		static bool supported() { return false; }

		base(core::polar *engine) : engine(engine) {}
		virtual ~base() {}

		inline void keep(core::ref r) {
			dtors.emplace_back(r);
		}

		virtual void init() {}
		virtual void update(DeltaTicks &) {}
		virtual void componentadded(IDType, std::type_index,
		                            std::weak_ptr<component::base>) {}
		virtual void componentremoved(IDType, std::type_index) {}
	};
} // namespace polar::system

#endif
