#pragma once

namespace polar {
namespace support {
	namespace input {
		enum class key : uint8_t {
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
			Backspace,
			Up,
			Down,
			Left,
			Right,
			MouseLeft,
			MouseMiddle,
			MouseRight,
			ControllerA,
			ControllerBack
		};

		struct key_hash : public std::unary_function<key, std::size_t> {
			std::size_t operator()(const key &k) const {
				return static_cast<size_t>(k);
			}
		};

		struct key_equal : public std::binary_function<key, key, bool> {
			bool operator()(const key &v0, const key &v1) const {
				return v0 == v1;
			}
		};
	}
}
}
