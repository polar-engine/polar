#pragma once

#include "System.h"
#include "Key.h"

typedef std::function<void(Key)> OnKeyHandler;
typedef std::function<void(Key, DeltaTicks &)> WhenKeyHandler;

class InputManager : public System {
private:
	std::vector<Key> keys;
	std::unordered_multimap<Key, OnKeyHandler> onKeyHandlers;
	std::unordered_multimap<Key, WhenKeyHandler> whenKeyHandlers;
public:
	static bool IsSupported() { return true; }
	InputManager(Polar *engine) : System(engine) {}
	void Init() override final;
	void Update(DeltaTicks &, std::vector<Object *> &) override final;
	void On(Key key, OnKeyHandler handler) {
		onKeyHandlers.emplace(key, handler);
	}
	void When(Key key, WhenKeyHandler handler) {
		whenKeyHandlers.emplace(key, handler);
	}
};
