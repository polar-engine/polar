#pragma once

#include <functional>
#include <polar/core/log.h>
#include <polar/core/serializer.h>
#include <map>

namespace polar {
	template<typename T> class wrapped_node;
} // namespace polar

namespace polar::component {
	class base;
} // namespace polar::component

namespace polar::core::registry {
	class component {
	  public:
		struct wrapper_type {
			std::shared_ptr<::polar::component::base> ptr;
			std::type_index ti;
		};

	  private:
		using entry_type = std::function<std::optional<wrapper_type>(store_deserializer &)>;
		using entries_type = std::unordered_map<std::string, entry_type>;

	  public:
		static entries_type & get() {
			static entries_type entries;
			return entries;
		}

		static bool reg(std::string name, entry_type entry) {
			get().emplace(name, entry);
			return true;
		}

		static std::optional<wrapper_type> deserialize(store_deserializer &s) {
			std::string name;
			s >> name;

			log()->trace("registry", "component name = ", name);

			auto &entries = get();

			auto it = entries.find(name);
			if(it != entries.end()) {
				return it->second(s);
			} else {
				log()->warning("registry", "no deserializer for component name `", name, '`');
				return {};
			}
		}
	};

	template<typename T>
	class node {
	  private:
		using entry_type = std::function<::polar::wrapped_node<T>(store_deserializer &)>;
		using entries_type = std::unordered_map<std::string, entry_type>;

	  public:
		static entries_type & get() {
			static entries_type entries;
			return entries;
		}

		static bool reg(std::string name, entry_type entry) {
			get().emplace(name, entry);
			return true;
		}

		static ::polar::wrapped_node<T> deserialize(store_deserializer &s) {
			uint8_t type;
			s >> type;

			if(type == 0) {
				T x;
				s >> x;
				return {x};
			} else {
				std::string name;
				s >> name;
				return get().at(name)(s);
			}
		}
	};
} // namespace polar::core::registry
