#pragma once

#include "Asset.h"
#include "FileSystem.h"

class AssetManager : public System {
public:
	static bool IsSupported() { return true; }
	AssetManager(const Polar *engine) : System(engine) {}

	template<typename T> std::string GetPath(const std::string &name) const {
		return "assets/" + T::Type() + '/' + name + ".asset";
	}

	template<typename T, typename ...Ts> T Get(const std::string name, Ts && ...args) const {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::Get requires object of type Asset");
		return T::Load(FileSystem::FileRead(GetPath<T>(name)), std::forward<Ts>(args)...);
	}

	template<typename T> T Get(const std::string name) const {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::Get requires object of type Asset");
		return T::Load(FileSystem::FileRead(GetPath<T>(name)));
	}
};
