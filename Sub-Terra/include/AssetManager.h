#pragma once

#include "Asset.h"
#include "FileSystem.h"

class AssetManager : public System {
public:
	static std::string GetAssetsDir() {
#ifdef _WIN32
		return FileSystem::GetAppDir() + "/assets";
#endif
#ifdef __APPLE__
		return FileSystem::GetAppDir() + "/Contents/Resources";
#endif
	}

	static bool IsSupported() { return true; }
	AssetManager(Polar *engine) : System(engine) {}

	template<typename T> std::string GetPath(const std::string &name) const {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::GetPath requires typename of type Asset");
		return GetAssetsDir() + '/' + AssetName<T>() + '/' + name + ".asset";
	}

	template<typename T, typename ...Ts> T Get(const std::string name, Ts && ...args) const {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::Get requires typename of type Asset");
		T asset(std::forward<Ts>(args)...);
		std::istringstream ss(FileSystem::ReadFile(GetPath<T>(name)));
		ss >> asset;
		return asset;
	}

	template<typename T> T Get(const std::string name) const {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::Get requires typename of type Asset");
		T asset;
		std::istringstream ss(FileSystem::ReadFile(GetPath<T>(name)));
		Deserializer(ss) >> asset;
		return asset;
	}
};
