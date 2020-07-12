#include <polar/core/log.h>
#include <polar/fs/local.h>
#include <polar/util/sdl.h>

namespace polar::core {
	std::shared_ptr<logger> logger::instance;

	void logger::msgbox(std::string title, std::string msg) {
#if defined(_WIN32)
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.data(), msg.data(), NULL);
#endif
		std::cerr << title << ": " << msg << std::endl;
		throw std::runtime_error("fatal error");
	}

	logger::logger(priority_t priority)
	  : file((fs::local::app_dir() / "log.txt").str(), std::ios::out | std::ios::binary | std::ios::trunc),
	    priority(priority) {}
}
