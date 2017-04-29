#pragma once

#include <unordered_map>
#include <steam/steam_api.h>
#include "System.h"

template<typename K, typename V> class ConfigManager : public System {
public:
	using HandlerTy = std::function<void(Polar *, K, V)>;
private:
	const V def;
	std::unordered_map<K, V> values;
	std::unordered_map<K, HandlerTy> handlers;
protected:
	void Init() override final {
		if(!SteamAPI_Init()) {
			ENGINE_THROW("failed to initialize Steam API");
		}
		INFOS("Welcome, " << SteamFriends()->GetPersonaName());
	}
public:
	static bool IsSupported() { return true; }
	ConfigManager(Polar *engine, V def) : System(engine), def(def) {}

	~ConfigManager() {
		SteamAPI_Shutdown();
	}

	void On(K k, HandlerTy h) {
		handlers[k] = h;
	}

	V Get(K k) {
		auto it = values.find(k);
		return it != values.cend() ? it->second : Set(k, def);
	}

	V Set(K k, V v) {
		values[k] = v;
		auto it = handlers.find(k);
		if(it != handlers.cend()) {
			it->second(engine, k, v);
		}
		return v;
	}

	void Save() {
		std::ostringstream oss;
		//oss << ??
	}
};
