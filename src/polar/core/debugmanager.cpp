#include <polar/core/debugmanager.h>
#include <polar/fs/local.h>
#include <polar/util/sdl.h>

namespace polar {
namespace core {
	std::shared_ptr<debugmanager_class> debugmanager_class::instance;

	void debugmanager_class::msgbox(std::string title, std::string msg) {
#if defined(_WIN32)
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.data(), msg.data(),
		                         NULL);
#endif
		std::cerr << title << ": " << msg << std::endl;
	}
	debugmanager_class::debugmanager_class(priority_t priority)
	    : file(fs::local::appdir() + "/log.txt",
	           std::ios::out | std::ios::binary | std::ios::trunc),
	      priority(priority) {}
}
}
