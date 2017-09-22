#pragma once

#include <set>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <polar/system/base.h>
#include <polar/support/event/arg.h>
#include <polar/support/input/key.h>

namespace polar { namespace system {
	typedef std::function<void(support::input::key)> OnKeyHandler;
	typedef std::function<void(support::input::key)> AfterKeyHandler;
	typedef std::function<void(support::input::key, const DeltaTicks &)> WhenKeyHandler;
	typedef std::function<void(const Point2 &)> MouseMoveHandler;
	typedef std::function<void(const Point2 &)> MouseWheelHandler;
	typedef std::function<void(const Point2 &)> ControllerAxesHandler;
	typedef std::function<void()> OnDigitalHandler;
	typedef std::function<void(const Point2 &)> OnAnalogHandler;

	class input : public base {
		using key_t = support::input::key;
		using arg_t = support::event::arg;
	public:
		typedef std::uint_fast32_t IDType;
		template<typename _Handler> using KeyHandlerBimap = boost::bimap<
			boost::bimaps::multiset_of<key_t>,
			boost::bimaps::unordered_set_of<IDType>,
			boost::bimaps::set_of_relation<>,
			boost::bimaps::with_info<_Handler>
		>;
		template<typename _Val> using IDMap = boost::bimap<
			boost::bimaps::set_of<IDType>,
			boost::bimaps::unconstrained_set_of<_Val>
		>;
		template<typename _Handler> using DigitalHandlerBimap = boost::bimap<
			boost::bimaps::multiset_of<std::string>,
			boost::bimaps::unordered_set_of<IDType>,
			boost::bimaps::set_of_relation<>,
			boost::bimaps::with_info<_Handler>
		>;
		template<typename _Handler> using AnalogHandlerBimap = boost::bimap<
			boost::bimaps::multiset_of<std::string>,
			boost::bimaps::unordered_set_of<IDType>,
			boost::bimaps::set_of_relation<>,
			boost::bimaps::with_info<_Handler>
		>;
	private:
		IDType nextID = 1;

		std::set<key_t> keys;
		KeyHandlerBimap<OnKeyHandler> onKeyHandlers;
		KeyHandlerBimap<AfterKeyHandler> afterKeyHandlers;
		KeyHandlerBimap<WhenKeyHandler> whenKeyHandlers;
		IDMap<MouseMoveHandler> mouseMoveHandlers;
		IDMap<MouseWheelHandler> mouseWheelHandlers;
		IDMap<ControllerAxesHandler> controllerAxesHandlers;

	protected:
		void init() override final;
		void update(DeltaTicks &) override final;
	public:
		const float controllerDeadZone = 0.05f;
		Point2 controllerAxes;
		std::set<std::string> trackedDigitals;
		std::set<std::string> trackedAnalogs;
		DigitalHandlerBimap<OnDigitalHandler> onDigitalHandlers;
		AnalogHandlerBimap<OnAnalogHandler> onAnalogHandlers;

		static bool supported() { return true; }
		input(core::polar *engine) : base(engine) {}

		inline std::shared_ptr<core::destructor> on(key_t key, const OnKeyHandler &handler) {
			auto id = nextID++;
			onKeyHandlers.insert(KeyHandlerBimap<OnKeyHandler>::value_type(key, id, handler));
			return std::make_shared<core::destructor>([this, id] () {
				onKeyHandlers.right.erase(id);
			});
		}

		inline std::shared_ptr<core::destructor> after(key_t key, const AfterKeyHandler &handler) {
			auto id = nextID++;
			afterKeyHandlers.insert(KeyHandlerBimap<AfterKeyHandler>::value_type(key, id, handler));
			return std::make_shared<core::destructor>([this, id] () {
				afterKeyHandlers.right.erase(id);
			});
		}

		inline std::shared_ptr<core::destructor> when(key_t key, const WhenKeyHandler &handler) {
			auto id = nextID++;
			whenKeyHandlers.insert(KeyHandlerBimap<WhenKeyHandler>::value_type(key, id, handler));
			return std::make_shared<core::destructor>([this, id] () {
				whenKeyHandlers.right.erase(id);
			});
		}

		inline std::shared_ptr<core::destructor> onmousemove(const MouseMoveHandler &handler) {
			auto id = nextID++;
			mouseMoveHandlers.insert(IDMap<MouseMoveHandler>::value_type(id, handler));
			return std::make_shared<core::destructor>([this, id] () {
				mouseMoveHandlers.left.erase(id);
			});
		}

		inline std::shared_ptr<core::destructor> onmousewheel(const MouseWheelHandler &handler) {
			auto id = nextID++;
			mouseWheelHandlers.insert(IDMap<MouseWheelHandler>::value_type(id, handler));
			return std::make_shared<core::destructor>([this, id] () {
				mouseWheelHandlers.left.erase(id);
			});
		}

		inline std::shared_ptr<core::destructor> oncontrolleraxes(const ControllerAxesHandler &handler) {
			auto id = nextID++;
			controllerAxesHandlers.insert(IDMap<ControllerAxesHandler>::value_type(id, handler));
			return std::make_shared<core::destructor>([this, id] () {
				controllerAxesHandlers.left.erase(id);
			});
		}

		inline std::shared_ptr<core::destructor> ondigital(std::string name, const OnDigitalHandler &handler) {
			auto id = nextID++;
			trackedDigitals.emplace(name);
			onDigitalHandlers.insert(DigitalHandlerBimap<OnDigitalHandler>::value_type(name, id, handler));
			return std::make_shared<core::destructor>([this, id] () {
				onDigitalHandlers.right.erase(id);
			});
		}

		inline std::shared_ptr<core::destructor> onanalog(std::string name, const OnAnalogHandler &handler) {
			auto id = nextID++;
			trackedAnalogs.emplace(name);
			onAnalogHandlers.insert(AnalogHandlerBimap<OnAnalogHandler>::value_type(name, id, handler));
			return std::make_shared<core::destructor>([this, id] () {
				onAnalogHandlers.right.erase(id);
			});
		}
	};
} }