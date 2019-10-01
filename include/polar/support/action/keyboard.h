#pragma once

#include <polar/support/input/key.h>
#include <polar/system/action.h>

namespace polar::support::action::keyboard {
	template<input::key K> struct key : system::action::digital {};

	inline std::type_index key_ti(input::key k) {
		using key_t = input::key;

#define CASE(K) case key_t::K: return typeid(key<key_t::K>);

		switch(k) {
			default:
				debugmanager()->warning("no matching type_index for key=", size_t(k));
				return typeid(key<key_t::None>);
			CASE(A)
			CASE(B)
			CASE(C)
			CASE(D)
			CASE(E)
			CASE(F)
			CASE(G)
			CASE(H)
			CASE(I)
			CASE(J)
			CASE(K)
			CASE(L)
			CASE(M)
			CASE(N)
			CASE(O)
			CASE(P)
			CASE(Q)
			CASE(R)
			CASE(S)
			CASE(T)
			CASE(U)
			CASE(V)
			CASE(W)
			CASE(X)
			CASE(Y)
			CASE(Z)
			CASE(Num1)
			CASE(Num2)
			CASE(Num3)
			CASE(Num4)
			CASE(Num5)
			CASE(Num6)
			CASE(Num7)
			CASE(Num8)
			CASE(Num9)
			CASE(Num0)
			CASE(Escape)
			CASE(Space)
			CASE(Enter)
			CASE(Backspace)
			CASE(Up)
			CASE(Down)
			CASE(Left)
			CASE(Right)
			CASE(Tilde)
			CASE(MouseLeft)
			CASE(MouseMiddle)
			CASE(MouseRight)
			CASE(ControllerA)
			CASE(ControllerBack)
		}

#undef CASE
	}
}
