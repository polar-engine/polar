#pragma once

#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <polar/support/event/arg.h>
#include <polar/system/base.h>
#include <unordered_map>

namespace polar {
namespace system {
	class event : public base {
		using arg_t = support::event::arg;

	  public:
		using global_listener_t =
		    std::function<void(const std::string &, arg_t)>;
		using listener_t = std::function<void(arg_t)>;
		using listener_bimap =
		    boost::bimap<boost::bimaps::multiset_of<std::string>,
		                 boost::bimaps::unordered_set_of<IDType>,
		                 boost::bimaps::set_of_relation<>,
		                 boost::bimaps::with_info<listener_t>>;

	  private:
		std::unordered_multimap<IDType, global_listener_t> globalListeners;
		listener_bimap listeners;
		IDType nextID = 1;

	  public:
		static bool supported() { return true; }
		event(core::polar *engine) : base(engine) {}

		inline std::shared_ptr<core::destructor>
		listen(const global_listener_t &fn) {
			auto id = nextID++;
			globalListeners.emplace(id, fn);
			return std::make_shared<core::destructor>(
			    [this, id]() { globalListeners.erase(id); });
		}

		inline std::shared_ptr<core::destructor>
		listenfor(const std::string &msg, const listener_t &fn) {
			return listenfor("", msg, fn);
		}

		inline std::shared_ptr<core::destructor>
		listenfor(const std::string &ns, const std::string &msg,
		          const listener_t &fn) {
			auto m  = ns + '.' + msg;
			auto id = nextID++;
			listeners.insert(listener_bimap::value_type(m, id, fn));
			return std::make_shared<core::destructor>(
			    [this, id]() { listeners.right.erase(id); });
		}

		inline void fire(const std::string &msg, arg_t arg = nullptr) const {
			firein("", msg, arg);
		}

		inline void firein(const std::string &ns, const std::string &msg,
		                   arg_t arg = nullptr) const {
			auto m     = ns + '.' + msg;
			auto range = listeners.left.equal_range(m);
			for(auto i = range.first; i != range.second; ++i) { i->info(arg); }
			for(auto &listener : globalListeners) { listener.second(m, arg); }
		}
	};
}
}
