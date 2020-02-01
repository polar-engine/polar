#pragma once

#include <polar/system/base.h>
#include <polar/util/enumclasshash.h>
#include <polar/util/getline.h>
#include <unordered_map>

namespace polar::system {
	template<typename K, typename FS> class config : public base {
	  public:
		using key_t     = K;
		using value_t   = math::decimal;
		using handler_t = std::function<void(core::polar *, key_t, value_t)>;

	  private:
		const std::string path;
		std::unordered_map<key_t, value_t, enum_class_hash> values;
		std::unordered_map<key_t, handler_t, enum_class_hash> handlers;

	  public:
		static bool supported() { return true; }
		config(core::polar *engine, std::string path)
		    : base(engine), path(path) {}
		~config() { save(); }

		void on(key_t k, handler_t h) { handlers[k] = h; }

		template<typename T> T get(key_t k) {
			auto it = values.find(k);
			return it != values.cend() ? T(it->second) : set<T>(k, T(0));
		}

		template<typename T> T set(key_t k, T v) {
			values[k] = value_t(v);
			auto it   = handlers.find(k);
			if(it != handlers.cend()) { it->second(engine, k, v); }
			return T(v);
		}

		void load() {
			if(!FS::exists(path)) {
				log()->verbose("config", "loading ", path, "... not found");
				return;
			}
			log()->verbose("config", "loading ", path, "... success");

			std::string s = FS::read(path);

			std::istringstream iss(s);
			std::string line;
			while(getline(iss, line)) {
				if(line == "") { continue; }

				std::istringstream issLine(line);
				key_t k = key_t(0);
				value_t v;
				issLine >> k >> v;
				set<value_t>(k, v);
			}
		}

		void save() const {
			std::ostringstream oss;
			for(auto &pair : values) {
				oss << pair.first << ' ' << pair.second << "\r\n";
			}
			auto result = FS::write(path, oss.str());
			log()->verbose("config", "saving ", path, "... ",
			                        (result ? "success" : "failed"));
		}
	};
} // namespace polar::system
