#pragma once

#include <string>
#include <boost/container/vector.hpp>

enum class StackActionType {
	Push,
	Pop
};

struct StackAction {
	StackActionType type;
	std::string name;
};

inline StackAction Push(std::string name) { return StackAction{StackActionType::Push, name}; }
inline StackAction Pop() { return StackAction{StackActionType::Pop}; }

typedef boost::container::vector<StackAction> Transition;
