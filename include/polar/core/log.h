#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <polar/core/types.h>
#include <polar/support/log/priority.h>
#include <string>
#include <vector>

namespace polar::core {
	inline std::ostream & operator<<(std::ostream &os, const Point2 &p) {
		return os << "Point2(" << p.x << ", " << p.y << ')';
	}
	
	inline std::ostream & operator<<(std::ostream &os, const Point3 &p) {
		return os << "Point3(" << p.x << ", " << p.y << ", " << p.z << ')';
	}

	inline std::ostream & operator<<(std::ostream &os, const Point3i &p) {
		return os << "Point3i(" << p.x << ", " << p.y << ", " << p.z << ')';
	}
	
	inline std::ostream & operator<<(std::ostream &os, const Point4 &p) {
		return os << "Point4(" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ')';
	}

	inline std::ostream & operator<<(std::ostream &os, const Quat &q) {
		return os << "Quat(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ')';
	}

	template<typename T>
	inline std::ostream & operator<<(std::ostream &os, const std::vector<T> &v) {
		os << '[';

		auto it = v.begin();
		if(it != v.end()) {
			os << *it;
			for(++it; it != v.end(); ++it) {
				os << ", " << *it;
			}
		}

		return os << ']';
	}

	class logger {
		using priority_t = support::debug::priority;

	  private:
		static std::shared_ptr<logger> instance;
		std::ofstream file;

	  public:
		priority_t priority;

		static inline auto get() {
			if(!instance) { instance = std::make_shared<logger>(); }
			return instance;
		}

		logger(priority_t priority = priority_t::info);

		void msgbox(std::string, std::string);

		template<typename T> inline void write(T arg) {
			std::cout << arg;
			file << arg;
		}

		inline void log_base(priority_t) {}
		template<typename T, typename... Ts>
		inline void log_base(priority_t p, T arg, Ts &&... args) {
			write(arg);
			log_base(p, std::forward<Ts>(args)...);
		}

		template<typename... Ts> inline void log(priority_t p, Ts &&... args) {
			static const std::array<std::string, size_t(priority_t::_size)>
			    uppers = {{"TRACE", "DEBUG", "VERBOSE", "INFO", "NOTICE",
			               "WARNING", "ERROR", "CRITICAL", "FATAL"}};

			if(p >= priority) {
				write('[');
				auto now = std::chrono::system_clock::now();
				auto tc = std::chrono::system_clock::to_time_t(now);
				auto lt = std::localtime(&tc);
				auto t = std::put_time(lt, "%T");
				write(t);
				write("] ");

				write('{');
				write(uppers[uint_fast8_t(p)]);
				write("} ");
				log_base(p, std::forward<Ts>(args)...);
#if defined(_WIN32)
				write('\r');
#endif
				write('\n');
			}

			switch(p) {
			default:
				break;
			case priority_t::fatal:
				msgbox("Fatal", "A fatal error has occurred. Please refer to "
				                "the engine log for details.");
				exit(1);
				break;
			}
		}

#ifdef _DEBUG
		template<typename... Ts> inline void trace(Ts &&... args) {
			log(priority_t::trace, std::forward<Ts>(args)...);
		}
#else
		// duplicated to avoid unused parameter warning
		template<typename... Ts> inline void trace(Ts &&...) {}
#endif
		template<typename... Ts> inline void debug(Ts &&... args) {
			log(priority_t::debug, std::forward<Ts>(args)...);
		}
		template<typename... Ts> inline void verbose(Ts &&... args) {
			log(priority_t::verbose, std::forward<Ts>(args)...);
		}
		template<typename... Ts> inline void info(Ts &&... args) {
			log(priority_t::info, std::forward<Ts>(args)...);
		}
		template<typename... Ts> inline void notice(Ts &&... args) {
			log(priority_t::notice, std::forward<Ts>(args)...);
		}
		template<typename... Ts> inline void warning(Ts &&... args) {
			log(priority_t::warning, std::forward<Ts>(args)...);
		}
		template<typename... Ts> inline void error(Ts &&... args) {
			log(priority_t::error, std::forward<Ts>(args)...);
		}
		template<typename... Ts> inline void critical(Ts &&... args) {
			log(priority_t::critical, std::forward<Ts>(args)...);
		}
		template<typename... Ts> inline void fatal(Ts &&... args) {
			log(priority_t::fatal, std::forward<Ts>(args)...);
		}
	};
} // namespace polar::core

namespace polar {
	inline auto log() { return core::logger::get(); }
} // namespace polar
