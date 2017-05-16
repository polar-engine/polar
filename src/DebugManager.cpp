#include "DebugManager.h"
#include "FileSystem.h"
#include "sdl.h"

std::shared_ptr<DebugManagerClass> DebugManagerClass::instance;

void DebugManagerClass::MsgBox(std::string title, std::string msg) {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.data(), msg.data(), NULL);
}


DebugManagerClass::DebugManagerClass(DebugPriority priority) :
	priority(priority),
	file(FileSystem::GetAppDir() + "/log.txt", std::ios::out | std::ios::binary | std::ios::trunc) {}
