#pragma once

#include <string>
#include <vector>

enum class StackActionType { Push, Pop, Quit };

struct StackAction {
	StackActionType type;
	std::string name;
};

inline auto Push(std::string name) {
	return StackAction{StackActionType::Push, name};
}
inline auto Pop() { return StackAction{StackActionType::Pop, ""}; }
inline auto QuitAction() { return StackAction{StackActionType::Quit, ""}; }

typedef std::vector<StackAction> Transition;
