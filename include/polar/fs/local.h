#pragma once

#include <fstream>
#include <polar/core/log.h>
#include <polar/core/path.h>
#include <polar/util/debug.h>
#include <sstream>
#include <string>
#include <vector>

namespace polar::fs {
	class local {
	  private:
		static void create_dir_impl(core::path path);

	  public:
		local() = delete;
		static core::path app();
		static core::path app_dir();
		static core::path saved_games_dir(std::string name);
		static std::string read(core::path path, size_t offset = 0, size_t len = 0, bool *eof = nullptr);
		static bool write(core::path path, std::istream &is);
		static uint64_t modified_time(core::path path);
		static std::vector<std::string> list_dir(core::path path);
		static void create_dir(core::path path);

		static inline bool write(core::path path, std::string s) {
			std::istringstream iss(s);
			return write(path, iss);
		}

		static inline bool exists(core::path path) {
			std::ifstream file(path.str());
			return file.good();
		}

		static inline void rename(core::path old_path, core::path new_path) {
			if(::rename(old_path.data(), new_path.data()) != 0) {
				log()->fatal("failed to rename `", old_path, "` to `", new_path, '`');
			}
		}

		static inline void remove_file(core::path path) {
			remove(path.data());
		}
	};
} // namespace polar::fs
