#pragma once

#include <unordered_map>
#include <polar/system/base.h>
#include <polar/asset/base.h>
#include <polar/fs/local.h>
#include <polar/core/serializer.h>
#include <polar/support/asset/partial.h>

namespace polar { namespace system {
	class asset : public base {
		using partial = support::asset::partial;
	private:
		std::unordered_map<std::string, partial> partials;
	public:
		static std::string getassetsdir() {
	#if defined(_WIN32) || defined(__linux__)
			return fs::local::appdir() + "/assets";
	#elif defined(__APPLE__)
			return fs::local::appdir() + "/Contents/Resources/assets";
	#endif
		}

		template<typename T> static std::string getdir() {
			static_assert(std::is_base_of<polar::asset::base, T>::value, "polar::system::asset::getdir requires typename of type polar::asset::base");
			return getassetsdir() + "/" + polar::asset::name<T>();
		}

		template<typename T> static std::string getpath(const std::string &name) {
			static_assert(std::is_base_of<polar::asset::base, T>::value, "polar::system::asset::getpath requires typename of type polar::asset::base");
			return getdir<T>() + "/" + name + ".asset";
		}

		template<typename T> std::vector<std::string> list() {
			static_assert(std::is_base_of<polar::asset::base, T>::value, "polar::system::asset::list requires typename of type polar::asset::base");
			auto ls = fs::local::listdir(getdir<T>());
			std::vector<std::string> filtered;
			for(auto &l : ls) {
				if(l.size() >= 6 && l.substr(l.size() - 6) == ".asset") {
					filtered.emplace_back(l.substr(0, l.size() - 6));
				}
			}
			return filtered;
		}

		static bool supported() { return true; }
		asset(core::polar *engine) : base(engine) {}

		void update(DeltaTicks &) override final {
			for(auto &pair : partials) {
				auto &partial = pair.second;
				if(!partial.done) {
					auto &path = pair.first;
					bool eof = false;
					partial.contents += fs::local::read(path, partial.contents.size(), 262144, &eof);
					if(eof) {
						partial.done = true;
						debugmanager()->verbose("async loaded asset ", path);
					}
				}
			}
		}

		template<typename T> void request(const std::string name) {
			static_assert(std::is_base_of<polar::asset::base, T>::value, "polar::system::asset::request requires typename of type polar::asset::base");
			auto path = getpath<T>(name);
			if(partials.find(path) == partials.end()) {
				partials.emplace(path, partial());
				debugmanager()->verbose("async loading asset ", path);
			}
		}

		template<typename T> partial & forcepartial(const std::string name) {
			static_assert(std::is_base_of<polar::asset::base, T>::value, "polar::system::asset::forcepartial requires typename of type polar::asset::base");
			auto path = getpath<T>(name);
			if(partials.find(path) == partials.end()) {
				debugmanager()->verbose("loading asset ", path);
				partials.emplace(path, partial());
			}
			auto &partial = partials[path];
			if(!partial.done) {
				auto test = partials[path];
				partial.contents += fs::local::read(path, partial.contents.size());
				partial.done = true;
				debugmanager()->verbose("loaded asset ", path);
			}
			return partial;
		}

		template<typename T, typename ...Ts> std::shared_ptr<T> get(const std::string name, Ts && ...args) {
			static_assert(std::is_base_of<polar::asset::base, T>::value, "polar::system::asset::get requires typename of type polar::asset::base");
			auto &partial = forcepartial<T>(name);
			if(!partial.asset) {
				debugmanager()->verbose("deserializing asset ", polar::asset::name<T>(), '/', name);

				// deserialize raw asset data to sub-class of polar::asset::base
				auto asset = std::make_shared<T>(std::forward<Ts>(args)...);
				std::istringstream ss(partial.contents);
				core::deserializer deserializer(ss);
				deserializer >> *asset;
				partial.asset = std::static_pointer_cast<polar::asset::base>(asset);

				// free up raw asset data
				std::string().swap(partial.contents);

				debugmanager()->verbose("deserialized asset ", polar::asset::name<T>(), '/', name);
			}
			return std::static_pointer_cast<T>(partial.asset);
		}
	};
} }
