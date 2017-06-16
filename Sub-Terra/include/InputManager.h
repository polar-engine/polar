#pragma once

#include <set>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <steam/steam_api.h>
#include "Destructor.h"
#include "System.h"
#include "Key.h"

typedef std::function<void(Key)> OnKeyHandler;
typedef std::function<void(Key)> AfterKeyHandler;
typedef std::function<void(Key, const DeltaTicks &)> WhenKeyHandler;
typedef std::function<void(const Point2 &)> MouseMoveHandler;
typedef std::function<void(const Point2 &)> MouseWheelHandler;
typedef std::function<void(const Point2 &)> ControllerAxesHandler;
typedef std::function<void()> OnDigitalHandler;
typedef std::function<void(const Point2 &)> OnAnalogHandler;

class InputManager : public System {
public:
	typedef std::uint_fast32_t IDType;
	template<typename _Handler> using KeyHandlerBimap = boost::bimap<
		boost::bimaps::multiset_of<Key>,
		boost::bimaps::unordered_set_of<IDType>,
		boost::bimaps::set_of_relation<>,
		boost::bimaps::with_info<_Handler>
	>;
	template<typename _Val> using IDMap = boost::bimap<
		boost::bimaps::set_of<IDType>,
		boost::bimaps::unconstrained_set_of<_Val>
	>;
	template<typename _Handler> using DigitalHandlerBimap = boost::bimap<
		boost::bimaps::multiset_of<ControllerDigitalActionHandle_t>,
		boost::bimaps::unordered_set_of<IDType>,
		boost::bimaps::set_of_relation<>,
		boost::bimaps::with_info<_Handler>
	>;
	template<typename _Handler> using AnalogHandlerBimap = boost::bimap<
		boost::bimaps::multiset_of<ControllerAnalogActionHandle_t>,
		boost::bimaps::unordered_set_of<IDType>,
		boost::bimaps::set_of_relation<>,
		boost::bimaps::with_info<_Handler>
	>;
private:
	IDType nextID = 1;

	std::set<Key> keys;
	KeyHandlerBimap<OnKeyHandler> onKeyHandlers;
	KeyHandlerBimap<AfterKeyHandler> afterKeyHandlers;
	KeyHandlerBimap<WhenKeyHandler> whenKeyHandlers;
	IDMap<MouseMoveHandler> mouseMoveHandlers;
	IDMap<MouseWheelHandler> mouseWheelHandlers;
	IDMap<ControllerAxesHandler> controllerAxesHandlers;

	ControllerActionSetHandle_t currentActionSet;
	Decimal currentSetAccum;
	std::set<ControllerDigitalActionHandle_t> trackedDigitals;
	std::set<ControllerDigitalActionHandle_t> digitals;
	DigitalHandlerBimap<OnDigitalHandler> onDigitalHandlers;
	std::set<ControllerAnalogActionHandle_t> trackedAnalogs;
	AnalogHandlerBimap<OnAnalogHandler> onAnalogHandlers;
protected:
	void Init() override final;
	void Update(DeltaTicks &) override final;
public:
	const float controllerDeadZone = 0.05f;
	Point2 controllerAxes;

	static bool IsSupported() { return true; }
	InputManager(Polar *engine) : System(engine) {}

	inline std::shared_ptr<Destructor> On(Key key, const OnKeyHandler &handler) {
		auto id = nextID++;
		onKeyHandlers.insert(KeyHandlerBimap<OnKeyHandler>::value_type(key, id, handler));
		return std::make_shared<Destructor>([this, id] () {
			onKeyHandlers.right.erase(id);
		});
	}

	inline std::shared_ptr<Destructor> After(Key key, const AfterKeyHandler &handler) {
		auto id = nextID++;
		afterKeyHandlers.insert(KeyHandlerBimap<AfterKeyHandler>::value_type(key, id, handler));
		return std::make_shared<Destructor>([this, id] () {
			afterKeyHandlers.right.erase(id);
		});
	}

	inline std::shared_ptr<Destructor> When(Key key, const WhenKeyHandler &handler) {
		auto id = nextID++;
		whenKeyHandlers.insert(KeyHandlerBimap<WhenKeyHandler>::value_type(key, id, handler));
		return std::make_shared<Destructor>([this, id] () {
			whenKeyHandlers.right.erase(id);
		});
	}

	inline std::shared_ptr<Destructor> OnMouseMove(const MouseMoveHandler &handler) {
		auto id = nextID++;
		mouseMoveHandlers.insert(IDMap<MouseMoveHandler>::value_type(id, handler));
		return std::make_shared<Destructor>([this, id] () {
			mouseMoveHandlers.left.erase(id);
		});
	}

	inline std::shared_ptr<Destructor> OnMouseWheel(const MouseWheelHandler &handler) {
		auto id = nextID++;
		mouseWheelHandlers.insert(IDMap<MouseWheelHandler>::value_type(id, handler));
		return std::make_shared<Destructor>([this, id] () {
			mouseWheelHandlers.left.erase(id);
		});
	}

	inline std::shared_ptr<Destructor> OnControllerAxes(const ControllerAxesHandler &handler) {
		auto id = nextID++;
		controllerAxesHandlers.insert(IDMap<ControllerAxesHandler>::value_type(id, handler));
		return std::make_shared<Destructor>([this, id] () {
			controllerAxesHandlers.left.erase(id);
		});
	}

	inline void SetActiveSet(std::string name) {
		currentSetAccum = 0.1;
		currentActionSet = SteamController()->GetActionSetHandle(name.data());
	}

	inline std::shared_ptr<Destructor> OnDigital(std::string name, const OnDigitalHandler &handler) {
		auto id = nextID++;
		ControllerDigitalActionHandle_t digital = SteamController()->GetDigitalActionHandle(name.data());
		trackedDigitals.emplace(digital);
		onDigitalHandlers.insert(DigitalHandlerBimap<OnDigitalHandler>::value_type(digital, id, handler));
		return std::make_shared<Destructor>([this, id] () {
			onDigitalHandlers.right.erase(id);
		});
	}

	inline std::shared_ptr<Destructor> OnAnalog(std::string name, const OnAnalogHandler &handler) {
		auto id = nextID++;
		ControllerAnalogActionHandle_t analog = SteamController()->GetAnalogActionHandle(name.data());
		trackedAnalogs.emplace(analog);
		onAnalogHandlers.insert(AnalogHandlerBimap<OnAnalogHandler>::value_type(analog, id, handler));
		return std::make_shared<Destructor>([this, id] () {
			onAnalogHandlers.right.erase(id);
		});
	}
};
