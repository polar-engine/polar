#pragma once

#include <string>
#include <vector>

enum class StackActionType { Push, Pop, Quit };

struct StackAction {
	StackActionType type;
	std::string name;
};

inline StackAction Push(std::string name) {
	return StackAction{StackActionType::Push, name};
}
inline StackAction Pop() { return StackAction{StackActionType::Pop, ""}; }
inline StackAction QuitAction() { return StackAction{StackActionType::Quit, ""}; }

typedef std::vector<StackAction> Transition;
