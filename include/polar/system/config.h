#pragma once

#include <unordered_map>
#include <steam/steam_api.h>
#include <polar/system/base.h>
#include <polar/util/getline.h>

struct EnumClassHash
{
	template <typename T>
	std::size_t operator()(T t) const
	{
		return static_cast<std::size_t>(t);
	}
};

template<typename K, typename FS> class ConfigManager : public System {
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
	~ConfigManager() { Save(); }

	void On(KeyTy k, HandlerTy h) {
		handlers[k] = h;
	}
	
	template<typename T> T Get(KeyTy k) {
		auto it = values.find(k);
		return it != values.cend() ? T(it->second) : Set<T>(k, T(0));
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
		if(!FS::Exists(path)) {
			DebugManager()->Verbose("loading ", path, "... not found");
			return;
		}
		DebugManager()->Verbose("loading ", path, "... success");

		std::string s = FS::Read(path);

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

	void Save() const {
		std::ostringstream oss;
		for(auto &pair : values) {
			oss << pair.first << ' ' << pair.second << "\r\n";
		}
		auto result = FS::Write(path, oss.str());
		DebugManager()->Verbose("saving ", path, "... ", (result ? "success" : "failed"));
	}
};
