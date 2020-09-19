#pragma once

#include <polar/asset/base.h>
#include <polar/core/path.h>
#include <polar/core/serializer.h>
#include <polar/fs/local.h>
#include <polar/support/asset/asset_ref.h>
#include <polar/support/asset/partial.h>
#include <polar/system/base.h>
#include <unordered_map>

namespace polar::system {
	class asset : public base {
		using partial                        = support::asset::partial;
		template<typename T> using asset_ref = support::asset::asset_ref<T>;

	  private:
		std::unordered_map<core::path, partial> partials;

		core::path assets_path = "";
		bool assets_path_valid = false;

	  public:
		core::path &get_assets_path() {
			if(!assets_path_valid) {
#if defined(_WIN32) || defined(__linux__)
				assets_path = fs::local::app_dir() / "assets";
#elif defined(__APPLE__)
				assets_path = fs::local::app_dir() / "Contents" / "Resources" / "assets";
#endif
				assets_path_valid = true;
			}
			return assets_path;
		}

		template<typename T> auto get_dir() {
			static_assert(std::is_base_of<polar::asset::base, T>::value,
			              "polar::system::asset::get_dir requires typename of type polar::asset::base");
			return get_assets_path() / polar::asset::name<T>();
		}

		template<typename T> auto get_path(std::string name) {
			static_assert(std::is_base_of<polar::asset::base, T>::value,
			              "polar::system::asset::get_path requires typename of type polar::asset::base");
			auto a = get_dir<T>();
			a /= name + ".asset";
			return a;
		}

		template<typename T> auto list() {
			static_assert(std::is_base_of<polar::asset::base, T>::value,
			              "polar::system::asset::list requires typename of "
			              "type polar::asset::base");
			auto ls = fs::local::list_dir(get_dir<T>());
			std::vector<std::string> filtered;
			for(auto &l : ls) {
				if(l.size() >= 6 && l.substr(l.size() - 6) == ".asset") {
					filtered.emplace_back(l.substr(0, l.size() - 6));
				}
			}
			return filtered;
		}

		static bool supported() { return true; }
		asset(core::polar &engine) : base(engine) {}

		virtual std::string name() const override { return "asset"; }

		void update(DeltaTicks &) override {
			for(auto &pair : partials) {
				auto &partial = pair.second;
				if(!partial.done) {
					auto &path = pair.first;
					bool eof   = false;
					partial.contents += fs::local::read(path, partial.contents.size(), 262144, &eof);
					if(eof) {
						partial.done = true;
						log()->verbose("asset", "async loaded asset ", path);
					}
				}
			}
		}

		template<typename T> void request(const std::string name) {
			static_assert(std::is_base_of<polar::asset::base, T>::value,
			              "polar::system::asset::request requires typename of "
			              "type polar::asset::base");
			auto path = get_path<T>(name);
			if(partials.find(path) == partials.end()) {
				partials.emplace(path, partial());
				log()->verbose("asset", "async loading asset ", path);
			}
		}

		template<typename T> partial &forcepartial(const std::string name) {
			static_assert(std::is_base_of<polar::asset::base, T>::value, "polar::system::asset::forcepartial requires "
			                                                             "typename of type polar::asset::base");
			auto path = get_path<T>(name);

			auto it = partials.find(path);
			if(it == partials.end()) {
				log()->verbose("asset", "loading asset ", path);
				auto [it2, r] = partials.emplace(path, partial());
				it            = it2;
			}
			auto &partial = it->second;
			if(!partial.done) {
				partial.contents += fs::local::read(path, partial.contents.size());
				partial.done = true;
				log()->verbose("asset", "loaded asset ", path);
			}
			return partial;
		}

		template<typename T, typename... Ts> std::shared_ptr<T> get(const std::string name, Ts &&... args) {
			static_assert(std::is_base_of<polar::asset::base, T>::value,
			              "polar::system::asset::get requires typename of type "
			              "polar::asset::base");
			auto &partial = forcepartial<T>(name);
			if(!partial.asset) {
				log()->verbose("asset", "deserializing asset ", polar::asset::name<T>(), '/', name);

				// deserialize raw asset data to sub-class of polar::asset::base
				auto asset = std::make_shared<T>(std::forward<Ts>(args)...);
				std::istringstream ss(partial.contents);
				core::deserializer deserializer(ss);
				deserializer >> *asset;
				partial.asset = std::static_pointer_cast<polar::asset::base>(asset);

				// free up raw asset data
				std::string().swap(partial.contents);

				log()->verbose("asset", "deserialized asset ", polar::asset::name<T>(), '/', name);
			}
			return std::static_pointer_cast<T>(partial.asset);
		}

		template<typename T, typename... Ts> std::shared_ptr<T> get(asset_ref<T> &ref, Ts &&... args) {
			std::shared_ptr<T> ptr;

			if(auto opt = ref.cache()) {
				ptr = *opt;
			} else {
				ptr = get<T>(ref.name(), std::forward<Ts>(args)...);
				ref.set(ptr);
			}
			return ptr;
		}
	};
} // namespace polar::system
