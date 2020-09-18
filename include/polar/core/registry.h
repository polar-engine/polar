#pragma once

#include <functional>
#include <polar/core/serializer.h>
#include <polar/node/base.h>
#include <map>

namespace polar {
	template<typename T> class wrapped_node;
} // namespace polar

namespace polar::core::registry {
	template<typename T>
	class node {
	  private:
		using entry_type = std::function<::polar::wrapped_node<T>(deserializer &)>;
		using entries_type = std::map<std::string, entry_type>;

	  public:
		static entries_type & get() {
			static entries_type entries;
			return entries;
		}

		static bool reg(std::string name, entry_type entry) {
			get().emplace(name, entry);
			return true;
		}

		static ::polar::wrapped_node<T> deserialize(deserializer &s) {
			std::string name;
			s >> name;

			if(name.empty()) {
				T x;
				s >> x;
				return {x};
			} else {
				return get().at(name)(s);
			}
		}
	};
} // namespace polar::core::registry
