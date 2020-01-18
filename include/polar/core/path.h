#pragma once

#include <boost/container/small_vector.hpp>
#include <polar/util/getline.h>
#include <sstream>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN

#if defined(NOMINMAX)
#include <Windows.h>
#else
#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX
#endif

#define POLAR_PATH_MAX MAX_PATH
#elif defined(_POSIX_VERSION)
#include <limits.h>
#define POLAR_PATH_MAX PATH_MAX
#endif

namespace polar::core {
	struct path_component {
		size_t index;
		size_t count;
	};

	class path {
	  private:
		boost::container::small_vector<path_component, 16> _components;
		std::string _str;
	  public:
		path(std::string str) {
			_str.reserve(POLAR_PATH_MAX);

			std::istringstream is(str);

			std::string component;
			while(get_component(is, component, {'\\', '/'})) {
				push_back(component);
			}
		}

		path(const char *sz) : path(std::string(sz)) {}

		const std::string & str() const {
			return _str;
		}

		const char * data() const {
			return str().data();
		}

		core::path dir() const {
			auto p = *this;
			p.pop_back();
			return p;
		}

		void push_back(std::string component) {
			if(!_components.empty()) {
				_str.push_back('/');
			}

			auto index = str().size();
			_str += component;

			_components.emplace_back(path_component{index, component.size()});
		}

		std::string pop_back() {
			auto back = _components.back();
			_components.pop_back();

			auto substr = _str.substr(back.index, back.count);
			_str.erase(back.index - 1, back.count + 1);

			return substr;
		}

		friend bool operator==(const path &lhs, const path &rhs) {
			return lhs.str() == rhs.str();
		}

		friend path operator/(const path &lhs, const std::string &rhs) {
			path p = lhs;
			p.push_back(rhs);
			return p;
		}

		friend path & operator/=(path &lhs, const std::string &rhs) {
			lhs.push_back(rhs);
			return lhs;
		}

		friend std::ostream & operator<<(std::ostream &os, const path &p) {
			return os << p.str();
		}
	};
} // namespace polar::core

namespace std {
	template<> struct hash<polar::core::path> {
		inline size_t operator()(const polar::core::path &p) const {
			return std::hash<std::string>()(p.str());
		}
	};
} // namespace std
