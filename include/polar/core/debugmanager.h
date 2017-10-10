#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <polar/support/debug/priority.h>
#include <string>
#include <vector>

namespace polar {
namespace core {
	class debugmanager_class {
		using priority_t = support::debug::priority;

	  private:
		static std::shared_ptr<debugmanager_class> instance;
		std::ofstream file;

	  public:
		priority_t priority;

		static std::shared_ptr<debugmanager_class> get() {
			if(!instance) { instance = std::make_shared<debugmanager_class>(); }
			return instance;
		}

		debugmanager_class(priority_t priority = priority_t::info);

		void msgbox(std::string, std::string);

		template <typename T> void write(T arg) {
			std::cout << arg;
			file << arg;
		}

		void log_base(priority_t) {}
		template <typename T, typename... Ts>
		void log_base(priority_t p, T arg, Ts &&... args) {
			write(arg);
			log_base(p, std::forward<Ts>(args)...);
		}

		template <typename... Ts> void log(priority_t p, Ts &&... args) {
			static const std::vector<std::string> uppers = {
			    "TRACE",   "DEBUG", "VERBOSE",  "INFO", "NOTICE",
			    "WARNING", "ERROR", "CRITICAL", "FATAL"};

			if(p >= priority) {
				write('[');
				write(uppers[uint_fast8_t(p)]);
				write("] ");
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

		template <typename... Ts> void trace(Ts &&... args) {
			log(priority_t::trace, std::forward<Ts>(args)...);
		}
		template <typename... Ts> void debug(Ts &&... args) {
			log(priority_t::debug, std::forward<Ts>(args)...);
		}
		template <typename... Ts> void verbose(Ts &&... args) {
			log(priority_t::verbose, std::forward<Ts>(args)...);
		}
		template <typename... Ts> void info(Ts &&... args) {
			log(priority_t::info, std::forward<Ts>(args)...);
		}
		template <typename... Ts> void notice(Ts &&... args) {
			log(priority_t::notice, std::forward<Ts>(args)...);
		}
		template <typename... Ts> void warning(Ts &&... args) {
			log(priority_t::warning, std::forward<Ts>(args)...);
		}
		template <typename... Ts> void error(Ts &&... args) {
			log(priority_t::error, std::forward<Ts>(args)...);
		}
		template <typename... Ts> void critical(Ts &&... args) {
			log(priority_t::critical, std::forward<Ts>(args)...);
		}
		template <typename... Ts> void fatal(Ts &&... args) {
			log(priority_t::fatal, std::forward<Ts>(args)...);
		}
	};
}
}

namespace polar {
inline std::shared_ptr<core::debugmanager_class> debugmanager() {
	return core::debugmanager_class::get();
}
}
