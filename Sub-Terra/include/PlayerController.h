#pragma once

#include "System.h"
#include <boost/array.hpp>

class PlayerController : public System {
public:
	typedef boost::array<IDType, 9> HotbarType;
protected:
	bool moveForward = false;
	bool moveBackward = false;
	bool moveLeft = false;
	bool moveRight = false;
	IDType object;
	HotbarType hotbar;
	HotbarType::size_type activeHotbar = 0;

	virtual void Init() override;
	virtual void Update(DeltaTicks &) override;
public:
	static bool IsSupported() { return true; }
	PlayerController(Polar *engine) : System(engine) {}
	~PlayerController() noexcept {
		engine->RemoveObject(object);
	}
};
