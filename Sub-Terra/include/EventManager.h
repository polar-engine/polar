#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include "Destructor.h"
#include "System.h"

union Arg {
	Decimal float_;
	void *pVoid;

	Arg(Decimal f) { float_ = f; }
	Arg(std::nullptr_t) { pVoid = nullptr; }
	template<typename T> Arg(T *p) { pVoid = reinterpret_cast<void *>(p); }
	template<typename T> inline T * Get() { return reinterpret_cast<T *>(pVoid); }
};

class EventManager : public System {
public:
	typedef std::function<void(const std::string &, Arg)> GlobalListener;
	typedef std::function<void(Arg)> Listener;
	typedef boost::bimap<
		boost::bimaps::multiset_of<std::string>,
		boost::bimaps::unordered_set_of<IDType>,
		boost::bimaps::set_of_relation<>,
		boost::bimaps::with_info<Listener>
	> ListenersBimap;
private:
	boost::unordered_multimap<IDType, GlobalListener> globalListeners;
	ListenersBimap listeners;
	IDType nextID = 1;
public:
	static bool IsSupported() { return true; }
	EventManager(Polar *engine) : System(engine) {}

	inline boost::shared_ptr<Destructor> Listen(const GlobalListener &fn) {
		auto id = nextID++;
		globalListeners.emplace(id, fn);
		return boost::make_shared<Destructor>([this, id] () {
			globalListeners.erase(id);
		});
	}

	inline boost::shared_ptr<Destructor> ListenFor(const std::string &msg, const Listener &fn) {
		return ListenFor("", msg, fn);
	}

	inline boost::shared_ptr<Destructor> ListenFor(const std::string &ns, const std::string &msg, const Listener &fn) {
		auto m = ns + '.' + msg;
		auto id = nextID++;
		listeners.insert(ListenersBimap::value_type(m, id, fn));
		return boost::make_shared<Destructor>([this, id] () {
			listeners.right.erase(id);
		});
	}

	inline void Fire(const std::string &msg, Arg arg = nullptr) const {
		FireIn("", msg, arg);
	}

	inline void FireIn(const std::string &ns, const std::string &msg, Arg arg = nullptr) const {
		auto m = ns + '.' + msg;
		auto range = listeners.left.equal_range(m);
		for(auto i = range.first; i != range.second; ++i) {
			i->info(arg);
		}
		for(auto &listener : globalListeners) {
			listener.second(m, arg);
		}
	}
};
