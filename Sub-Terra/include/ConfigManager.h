#pragma once

#include <unordered_map>
#include <steam/steam_api.h>
#include "System.h"
#include "getline.h"

template<typename K, typename V> class ConfigManager : public System {
public:
	using HandlerTy = std::function<void(Polar *, K, V)>;
private:
	const std::string path;
	const V def;
	std::unordered_map<K, V> values;
	std::unordered_map<K, HandlerTy> handlers;
protected:
	void Init() override final {

	}
public:
	static bool IsSupported() { return true; }
	ConfigManager(Polar *engine, std::string path, V def) : System(engine), path(path), def(def) {}

	~ConfigManager() {
		Save();
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

	void Load() {
		if(!SteamRemoteStorage()->FileExists(path.data())) {
			INFOS("loading " << path << "... not found");
			return;
		}

		INFOS("loading " << path << "... success");

		int32_t size = SteamRemoteStorage()->GetFileSize(path.data());
		char *sz = new char[size];
		SteamRemoteStorage()->FileRead(path.data(), sz, size);
		std::string s(sz, size);
		delete[] sz;

		std::istringstream iss(s);
		std::string line;
		while(getline(iss, line)) {
			std::istringstream issLine(line);
			K k = K(0);
			V v;
			issLine >> k >> v;
			Set(k, v);
		}
	}

	void Save() {
		std::ostringstream oss;
		for(auto &pair : values) {
			oss << pair.first << ' ' << pair.second << std::endl;
		}
		auto s = oss.str();
		auto result = SteamRemoteStorage()->FileWrite(path.data(), s.data(), s.size());
		INFOS("saving " << path << "... " << (result ? "success" : "failed"));
	}
};
