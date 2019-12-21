#pragma once

#include <polar/util/getline.h>
#include <sstream>

namespace polar::core {
	class path {
	  private:
		std::vector<std::string> _components;
		std::string _str;

		void regen_str() {
			std::ostringstream os;
			for(size_t i = 0; i < _components.size(); ++i) {
				if(i > 0) {
					os << '/';
				}
				os << _components[i];
			}
			_str = os.str();
		}
	  public:
		path(std::string str) {
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

		void push_back(std::string s) {
			_components.emplace_back(s);
			regen_str();
		}

		std::string pop_back() {
			auto back = _components.back();
			_components.pop_back();
			regen_str();
			return back;
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
