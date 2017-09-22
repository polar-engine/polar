#pragma once

#include <SDL.h>
#include <polar/core/debugmanager.h>
#include <polar/support/input/key.h>
#include <polar/util/debug.h>

/* SDL defines main to be SDL_main which is annoying */
#ifdef main
#undef main
#endif

inline bool _SDL_real(const char *file, const long line, const char *code) {
	const char *err = SDL_GetError();
	SDL_ClearError();
	if(err[0] != '\0') {
		polar::debugmanager()->warning("SDL: ", err);
		polar::debugmanager()->debug(file, ' ', line);
		polar::debugmanager()->trace(code);
	}
	return err[0] == '\0';
}

#define ENGINE_SDL(CODE) ((CODE), _SDL_real(BASEFILE, __LINE__, #CODE))
#define SDL ENGINE_SDL

#ifdef _DEBUG
#define IGNORE_SDL(CODE) ((CODE), SDL_ClearError())
#else
#define IGNORE_SDL(CODE) (CODE)
#endif

inline polar::support::input::key mkKeyFromSDL(SDL_Keycode k) {
	using key = polar::support::input::key;

	switch(k) {
	case SDLK_a:
		return key::A;
	case SDLK_b:
		return key::B;
	case SDLK_c:
		return key::C;
	case SDLK_d:
		return key::D;
	case SDLK_e:
		return key::E;
	case SDLK_f:
		return key::F;
	case SDLK_g:
		return key::G;
	case SDLK_h:
		return key::H;
	case SDLK_i:
		return key::I;
	case SDLK_j:
		return key::J;
	case SDLK_k:
		return key::K;
	case SDLK_l:
		return key::L;
	case SDLK_m:
		return key::M;
	case SDLK_n:
		return key::N;
	case SDLK_o:
		return key::O;
	case SDLK_p:
		return key::P;
	case SDLK_q:
		return key::Q;
	case SDLK_r:
		return key::R;
	case SDLK_s:
		return key::S;
	case SDLK_t:
		return key::T;
	case SDLK_u:
		return key::U;
	case SDLK_v:
		return key::V;
	case SDLK_w:
		return key::W;
	case SDLK_x:
		return key::X;
	case SDLK_y:
		return key::Y;
	case SDLK_z:
		return key::Z;
	case SDLK_1:
		return key::Num1;
	case SDLK_2:
		return key::Num2;
	case SDLK_3:
		return key::Num3;
	case SDLK_4:
		return key::Num4;
	case SDLK_5:
		return key::Num5;
	case SDLK_6:
		return key::Num6;
	case SDLK_7:
		return key::Num7;
	case SDLK_8:
		return key::Num8;
	case SDLK_9:
		return key::Num9;
	case SDLK_0:
		return key::Num0;
	case SDLK_ESCAPE:
		return key::Escape;
	case SDLK_SPACE:
		return key::Space;
	case SDLK_RETURN:
		return key::Enter;
	case SDLK_BACKSPACE:
		return key::Backspace;
	case SDLK_UP:
		return key::Up;
	case SDLK_DOWN:
		return key::Down;
	case SDLK_LEFT:
		return key::Left;
	case SDLK_RIGHT:
		return key::Right;
	default:
		return key::None;
	}
}

inline polar::support::input::key mkMouseButtonFromSDL(Uint8 mb) {
	using key = polar::support::input::key;

	switch(mb) {
	case SDL_BUTTON_LEFT:
		return key::MouseLeft;
	case SDL_BUTTON_MIDDLE:
		return key::MouseMiddle;
	case SDL_BUTTON_RIGHT:
		return key::MouseRight;
	default:
		return key::None;
	}
}

inline polar::support::input::key mkButtonFromSDL(SDL_GameControllerButton b) {
	using key = polar::support::input::key;

	switch(b) {
	case SDL_CONTROLLER_BUTTON_A:
		return key::ControllerA;
	case SDL_CONTROLLER_BUTTON_BACK:
		return key::ControllerBack;
	default:
		return key::None;
	}
}
