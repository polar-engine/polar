#pragma once

#include "System.h"
#include "Key.h"

typedef std::function<void(Key)> OnKeyHandler;
typedef std::function<void(Key, const DeltaTicks &)> WhenKeyHandler;
typedef std::function<void(const Point2 &)> MouseMoveHandler;

class InputManager : public System {
private:
	std::vector<Key> keys;
	std::unordered_multimap<Key, OnKeyHandler> onKeyHandlers;
	std::unordered_multimap<Key, WhenKeyHandler> whenKeyHandlers;
	std::vector<MouseMoveHandler> mouseMoveHandlers;
protected:
	void Init() override final;
	void Update(DeltaTicks &, std::vector<Object *> &) override final;
public:
	static bool IsSupported() { return true; }
	InputManager(Polar *engine) : System(engine) {}
	void On(Key key, const OnKeyHandler &handler) {
		onKeyHandlers.emplace(key, handler);
	}
	void When(Key key, const WhenKeyHandler &handler) {
		whenKeyHandlers.emplace(key, handler);
	}
	void OnMouseMove(const MouseMoveHandler &handler) {
		mouseMoveHandlers.emplace_back(handler);
	}
};
