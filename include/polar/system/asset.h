#pragma once

#include <unordered_map>
#include <polar/system/base.h>
#include <polar/asset/base.h>
#include <polar/fs/local.h>
#include <polar/core/serializer.h>

struct PartialAsset {
	bool done = false;
	std::string contents;
	std::shared_ptr<Asset> asset;
};

class AssetManager : public System {
private:
	std::unordered_map<std::string, PartialAsset> partials;
public:
	static std::string GetAssetsDir() {
#if defined(_WIN32)
		return FileSystem::AppDir() + "/assets";
#elif defined(__APPLE__)
		return FileSystem::AppDir() + "/Contents/Resources/assets";
#endif
	}

	template<typename T> static std::string GetDir() {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::GetDir requires typename of type Asset");
		return GetAssetsDir() + "/" + AssetName<T>();
	}

	template<typename T> static std::string GetPath(const std::string &name) {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::GetPath requires typename of type Asset");
		return GetDir<T>() + "/" + name + ".asset";
	}

	template<typename T> std::vector<std::string> List() {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::List requires typename of type Asset");
		auto ls = FileSystem::ListDir(GetDir<T>());
		std::vector<std::string> filtered;
		for(auto &l : ls) {
			if(l.size() >= 6 && l.substr(l.size() - 6) == ".asset") {
				filtered.emplace_back(l.substr(0, l.size() - 6));
			}
		}
		return filtered;
	}

	static bool IsSupported() { return true; }
	AssetManager(Polar *engine) : System(engine) {}

	void Update(DeltaTicks &) override final {
		for(auto &pair : partials) {
			auto &partial = pair.second;
			if(!partial.done) {
				auto &path = pair.first;
				bool eof = false;
				partial.contents += FileSystem::Read(path, partial.contents.size(), 262144, &eof);
				if(eof) {
					partial.done = true;
					DebugManager()->Verbose("async loaded asset ", path);
				}
			}
		}
	}

	template<typename T> void Request(const std::string name) {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::GetString requires typename of type Asset");
		auto path = GetPath<T>(name);
		if(partials.find(path) == partials.end()) {
			partials.emplace(path, PartialAsset());
			DebugManager()->Verbose("async loading asset ", path);
		}
	}

	template<typename T> PartialAsset & ForcePartial(const std::string name) {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::GetString requires typename of type Asset");
		auto path = GetPath<T>(name);
		if(partials.find(path) == partials.end()) {
			DebugManager()->Verbose("loading asset ", path);
			partials.emplace(path, PartialAsset());
		}
		auto &partial = partials[path];
		if(!partial.done) {
			auto test = partials[path];
			partial.contents += FileSystem::Read(path, partial.contents.size());
			partial.done = true;
			DebugManager()->Verbose("loaded asset ", path);
		}
		return partial;
	}

	template<typename T, typename ...Ts> std::shared_ptr<T> Get(const std::string name, Ts && ...args) {
		static_assert(std::is_base_of<Asset, T>::value, "AssetManager::Get requires typename of type Asset");
		auto &partial = ForcePartial<T>(name);
		if(!partial.asset) {
			DebugManager()->Verbose("deserializing asset ", AssetName<T>(), '/', name);

			// deserialize raw asset data to sub-class of Asset
			auto asset = std::make_shared<T>(std::forward<Ts>(args)...);
			std::istringstream ss(partial.contents);
			Deserializer deserializer(ss);
			deserializer >> *asset;
			partial.asset = std::static_pointer_cast<Asset>(asset);

			// free up raw asset data
			std::string().swap(partial.contents);

			DebugManager()->Verbose("deserialized asset ", AssetName<T>(), '/', name);
		}
		return std::static_pointer_cast<T>(partial.asset);
	}
};
