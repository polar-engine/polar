#pragma once

#include <vector>
#include <string>
#include <memory>
#include <iostream>

enum class DebugPriority : uint_fast8_t {
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

class DebugManagerClass {
private:
	static std::shared_ptr<DebugManagerClass> instance;
public:
	DebugPriority priority;

	static std::shared_ptr<DebugManagerClass> Get() {
		if(!instance) { instance = std::make_shared<DebugManagerClass>(); }
		return instance;
	}

	DebugManagerClass(DebugPriority priority = DebugPriority::Info) : priority(priority) {}

	void MsgBox(std::string, std::string);

	template<typename T> void Write(T arg) {
		std::cout << arg;
	}

	void LogBase(DebugPriority) {}

	template<typename T, typename ...Ts> void LogBase(DebugPriority p, T arg, Ts && ...args) {
		std::cout << arg;
		LogBase(p, std::forward<Ts>(args)...);
	}

	template<typename ...Ts> void Log(DebugPriority p, Ts && ...args) {
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

		switch(p) {
		case DebugPriority::Fatal:
			MsgBox("Fatal", "A fatal error has occurred. Please refer to the engine log for details.");
			exit(1);
			break;
		}
	}

	template<typename ...Ts> void Trace   (Ts && ...args) { Log(DebugPriority::Trace,    std::forward<Ts>(args)...); }
	template<typename ...Ts> void Debug   (Ts && ...args) { Log(DebugPriority::Debug,    std::forward<Ts>(args)...); }
	template<typename ...Ts> void Verbose (Ts && ...args) { Log(DebugPriority::Verbose,  std::forward<Ts>(args)...); }
	template<typename ...Ts> void Info    (Ts && ...args) { Log(DebugPriority::Info,     std::forward<Ts>(args)...); }
	template<typename ...Ts> void Notice  (Ts && ...args) { Log(DebugPriority::Notice,   std::forward<Ts>(args)...); }
	template<typename ...Ts> void Warning (Ts && ...args) { Log(DebugPriority::Warning,  std::forward<Ts>(args)...); }
	template<typename ...Ts> void Error   (Ts && ...args) { Log(DebugPriority::Error,    std::forward<Ts>(args)...); }
	template<typename ...Ts> void Critical(Ts && ...args) { Log(DebugPriority::Critical, std::forward<Ts>(args)...); }
	template<typename ...Ts> void Fatal   (Ts && ...args) { Log(DebugPriority::Fatal,    std::forward<Ts>(args)...); }
};

inline std::shared_ptr<DebugManagerClass> DebugManager() {
	return DebugManagerClass::Get();
}
