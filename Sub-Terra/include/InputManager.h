#pragma once

#include <set>
#include "System.h"
#include "Key.h"

typedef std::function<void(Key)> OnKeyHandler;
typedef std::function<void(Key)> AfterKeyHandler;
typedef std::function<void(Key, const DeltaTicks &)> WhenKeyHandler;
typedef std::function<void(const Point2 &)> MouseMoveHandler;

class InputManager : public System {
private:
	std::set<Key> keys;
	std::unordered_multimap<Key, OnKeyHandler, KeyHash, KeyEqual> onKeyHandlers;
	std::unordered_multimap<Key, AfterKeyHandler, KeyHash, KeyEqual> afterKeyHandlers;
	std::unordered_multimap<Key, WhenKeyHandler, KeyHash, KeyEqual> whenKeyHandlers;
	std::vector<MouseMoveHandler> mouseMoveHandlers;
protected:
	void Init() override final;
	void Update(DeltaTicks &) override final;
public:
	static bool IsSupported() { return true; }
	InputManager(Polar *engine) : System(engine) {}
	void On(Key key, const OnKeyHandler &handler) {
		onKeyHandlers.emplace(key, handler);
	}
	void After(Key key, const AfterKeyHandler &handler) {
		afterKeyHandlers.emplace(key, handler);
	}
	void When(Key key, const WhenKeyHandler &handler) {
		whenKeyHandlers.emplace(key, handler);
	}
	void OnMouseMove(const MouseMoveHandler &handler) {
		mouseMoveHandlers.emplace_back(handler);
	}
};
