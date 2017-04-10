#pragma once

enum class Key : uint8_t {
	None,
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	Num1,
	Num2,
	Num3,
	Num4,
	Num5,
	Num6,
	Num7,
	Num8,
	Num9,
	Num0,
	Escape,
	Space,
	Enter,
	Up,
	Down,
	Left,
	Right,
	ControllerA,
	ControllerBack
};

struct KeyHash : public std::unary_function<Key, std::size_t> {
	std::size_t operator()(const Key &k) const {
		return static_cast<size_t>(k);
	}
};

struct KeyEqual : public std::binary_function<Key, Key, bool> {
	bool operator()(const Key &v0, const Key &v1) const {
		return v0 == v1;
	}
};
