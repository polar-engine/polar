#pragma once

#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <polar/core/id.h>
#include <polar/support/event/arg.h>
#include <polar/system/base.h>
#include <unordered_map>

namespace polar::system {
	class event : public base {
		using arg_t = support::event::arg;

	  public:
		using global_listener_t = std::function<void(const std::string &, arg_t)>;
		using listener_t = std::function<void(arg_t)>;
		using listener_bimap = boost::bimap<
			boost::bimaps::multiset_of<std::string>,
			boost::bimaps::unordered_set_of<core::id>,
			boost::bimaps::set_of_relation<>,
			boost::bimaps::with_info<listener_t>
		>;

	  private:
		std::unordered_multimap<core::id, global_listener_t> globalListeners;
		listener_bimap listeners;
		core::id nextID = 1;

	  public:
		static bool supported() { return true; }
		event(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "event"; }

		inline auto listen(const global_listener_t &fn) {
			auto id = nextID++;
			globalListeners.emplace(id, fn);
			return core::ref(
			    [this, id]() { globalListeners.erase(id); });
		}

		inline auto listenfor(const std::string &ns, const std::string &msg,
		                      const listener_t &fn) {
			auto m  = ns + '.' + msg;
			auto id = nextID++;
			listeners.insert(listener_bimap::value_type(m, id, fn));
			return core::ref(
			    [this, id]() { listeners.right.erase(id); });
		}

		inline auto listenfor(const std::string &msg, const listener_t &fn) {
			return listenfor("", msg, fn);
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
} // namespace polar::system
