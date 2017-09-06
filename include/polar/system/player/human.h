#pragma once

#include <array>
#include <polar/system/base.h>
#include <polar/support/input/key.h>

namespace polar { namespace system { namespace player {
	class human : public base {
		using key_t = support::input::key;
	private:
		std::shared_ptr<core::destructor> timeDtor;
		IDType timeID = 0;

		// the size of the array determines how many concurrent sounds we can play at once
		std::array<std::shared_ptr<core::destructor>, 4> soundDtors;
		size_t soundIndex = 0;

		IDType object;
		Point2 orientVel;
		Point2 orientRot;
		Decimal velocity = 10.0;
	protected:
		virtual void init() override;
		virtual void update(DeltaTicks &) override;
	public:
		Decimal oldTime = 0;
		Decimal smoothing = Decimal(0.995);

		static bool supported() { return true; }
		human(core::polar *engine, const IDType object) : base(engine), object(object) {}
	};
} } }
