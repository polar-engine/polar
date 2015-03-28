#pragma once

#include <boost/container/set.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include "System.h"
#include "Key.h"

typedef std::function<void(Key)> OnKeyHandler;
typedef std::function<void(Key)> AfterKeyHandler;
typedef std::function<void(Key, const DeltaTicks &)> WhenKeyHandler;
typedef std::function<void(const Point2 &)> MouseMoveHandler;

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
private:
	boost::container::set<Key> keys;
	KeyHandlerBimap<OnKeyHandler> onKeyHandlers;
	KeyHandlerBimap<AfterKeyHandler> afterKeyHandlers;
	KeyHandlerBimap<WhenKeyHandler> whenKeyHandlers;
	IDMap<MouseMoveHandler> mouseMoveHandlers;
	IDType nextID = 1;
protected:
	void Init() override final;
	void Update(DeltaTicks &) override final;
public:
	static bool IsSupported() { return true; }
	InputManager(Polar *engine) : System(engine) {}
	void On(Key key, const OnKeyHandler &handler) {
		onKeyHandlers.insert(KeyHandlerBimap<OnKeyHandler>::value_type(key, nextID++, handler));
	}
	void After(Key key, const AfterKeyHandler &handler) {
		afterKeyHandlers.insert(KeyHandlerBimap<AfterKeyHandler>::value_type(key, nextID++, handler));
	}
	void When(Key key, const WhenKeyHandler &handler) {
		whenKeyHandlers.insert(KeyHandlerBimap<WhenKeyHandler>::value_type(key, nextID++, handler));
	}
	void OnMouseMove(const MouseMoveHandler &handler) {
		mouseMoveHandlers.insert(IDMap<MouseMoveHandler>::value_type(nextID++, handler));
	}
};
