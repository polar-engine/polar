#pragma once

#include "Asset.h"

class AssetManager : public System {
public:
	AssetManager(Polar *engine) : System(engine) {}
	std::string Load(const std::string name) {
		std::ifstream file(name, std::ios::in | std::ios::binary | std::ios::ate);
		if(file) {
			std::streamoff len = file.tellg();
			file.seekg(0, std::ios::beg);
			char *sz = new char[static_cast<unsigned int>(len + 1)];
			sz[len] = '\0';
			file.read(sz, static_cast<unsigned int>(len));
			return std::string(sz);
		}
	}
	template<typename T, typename ...Ts> T Get(const std::string name, Ts && ...args) {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::Get requires object of type Asset");
		return T(Load(name), std::forward<Ts>(args)...);
	}

	template<typename T> T Get(const std::string name) {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::Get requires object of type Asset");
		return T(Load(name));
	}
};
