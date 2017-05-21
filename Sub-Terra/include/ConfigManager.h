#pragma once

#include <unordered_map>
#include <steam/steam_api.h>
#include "System.h"
#include "getline.h"

struct EnumClassHash
{
	template <typename T>
	std::size_t operator()(T t) const
	{
		return static_cast<std::size_t>(t);
	}
};

template<typename K> class ConfigManager : public System {
public:
	using KeyTy = K;
	using ValueTy = Decimal;
	using HandlerTy = std::function<void(Polar *, KeyTy, ValueTy)>;
private:
	const std::string path;
	std::unordered_map<KeyTy, ValueTy, EnumClassHash> values;
	std::unordered_map<KeyTy, HandlerTy, EnumClassHash> handlers;
public:
	static bool IsSupported() { return true; }
	ConfigManager(Polar *engine, std::string path) : System(engine), path(path) {}

	~ConfigManager() {
		Save();
	}

	void On(KeyTy k, HandlerTy h) {
		handlers[k] = h;
	}
	
	template<typename T> T Get(KeyTy k) {
		auto it = values.find(k);
		auto ret = it != values.cend() ? T(it->second) : Set<T>(k, T(0));
		return ret;
	}

	template<typename T> T Set(KeyTy k, T v) {
		values[k] = ValueTy(v);
		auto it = handlers.find(k);
		if(it != handlers.cend()) {
			it->second(engine, k, v);
		}
		return T(v);
	}

	void Load() {
		if(!SteamRemoteStorage()->FileExists(path.data())) {
			DebugManager()->Verbose("loading ", path, "... not found");
			return;
		}

		DebugManager()->Verbose("loading ", path, "... success");

		int32_t size = SteamRemoteStorage()->GetFileSize(path.data());
		char *sz = new char[size];
		SteamRemoteStorage()->FileRead(path.data(), sz, size);
		std::string s(sz, size);
		delete[] sz;

		std::istringstream iss(s);
		std::string line;
		while(getline(iss, line)) {
			if(line == "") { continue; }

			std::istringstream issLine(line);
			KeyTy k = KeyTy(0);
			ValueTy v;
			issLine >> k >> v;
			Set<ValueTy>(k, v);
		}
	}

	void Save() {
		std::ostringstream oss;
		for(auto &pair : values) {
			oss << pair.first << ' ' << pair.second << "\r\n";
		}
		auto s = oss.str();
		auto result = SteamRemoteStorage()->FileWrite(path.data(), s.data(), s.size());
		DebugManager()->Verbose("saving ", path, "... ", (result ? "success" : "failed"));
	}
};
