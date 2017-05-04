#pragma once

#include <vector>
#include <string>
#include <iostream>

class DebugManager {
public:
	enum class Priority : uint_fast8_t {
		Trace,
		Debug,
		Verbose,
		Info,
		Notice,
		Warning,
		Error,
		Critical,
		Fatal
	};

	Priority priority;

	DebugManager(Priority priority) : priority(priority) {}

	template<typename T> void Write(T arg) {
		std::cout << arg;
	}

	void LogBase(Priority) {}

	template<typename T, typename ...Ts> void LogBase(Priority p, T arg, Ts && ...args) {
		std::cout << arg;
		LogBase(p, std::forward<Ts>(args)...);
	}

	template<typename ...Ts> void Log(Priority p, Ts && ...args) {
		static const std::vector<std::string> uppers = {
			"TRACE",
			"DEBUG",
			"VERBOSE",
			"INFO",
			"NOTICE",
			"WARNING",
			"ERROR",
			"CRITICAL",
			"FATAL"
		};

		if(p >= priority) {
			Write('[');
			Write(uppers[uint_fast8_t(p)]);
			Write("] ");
			LogBase(p, std::forward<Ts>(args)...);
			Write('\n');
		}
	}

	template<typename ...Ts> void Trace   (Ts && ...args) { Log(Priority::Trace,    std::forward<Ts>(args)...); }
	template<typename ...Ts> void Debug   (Ts && ...args) { Log(Priority::Debug,    std::forward<Ts>(args)...); }
	template<typename ...Ts> void Verbose (Ts && ...args) { Log(Priority::Verbose,  std::forward<Ts>(args)...); }
	template<typename ...Ts> void Info    (Ts && ...args) { Log(Priority::Info,     std::forward<Ts>(args)...); }
	template<typename ...Ts> void Notice  (Ts && ...args) { Log(Priority::Notice,   std::forward<Ts>(args)...); }
	template<typename ...Ts> void Warning (Ts && ...args) { Log(Priority::Warning,  std::forward<Ts>(args)...); }
	template<typename ...Ts> void Error   (Ts && ...args) { Log(Priority::Error,    std::forward<Ts>(args)...); }
	template<typename ...Ts> void Critical(Ts && ...args) { Log(Priority::Critical, std::forward<Ts>(args)...); }
	template<typename ...Ts> void Fatal   (Ts && ...args) { Log(Priority::Fatal,    std::forward<Ts>(args)...); }
};
