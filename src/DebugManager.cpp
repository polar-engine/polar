#include "DebugManager.h"
#include "sdl.h"

std::shared_ptr<DebugManagerClass> DebugManagerClass::instance;

void DebugManagerClass::MsgBox(std::string title, std::string msg) {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.data(), msg.data(), NULL);
}
