#pragma once

#include <fstream>
#include <polar/core/debugmanager.h>
#include <polar/util/debug.h>
#include <sstream>
#include <string>
#include <vector>

namespace polar::fs {
	class local {
	  private:
		static void createdir_impl(std::string path);

	  public:
		local() = delete;
		static std::string app();
		static std::string appdir();
		static std::string savedgamesdir(std::string name);
		static std::string read(std::string path, size_t offset = 0, size_t len = 0, bool *eof = nullptr);
		static bool write(std::string path, std::istream &is);
		static uint64_t modifiedtime(std::string path);
		static std::vector<std::string> listdir(std::string path);
		static void createdir(std::string path);

		static inline std::string dir_of(std::string path) {
			std::string::size_type pos = path.find_last_of("\\/");
			return path.substr(0, pos);
		}

		static inline bool write(std::string path, std::string s) {
			std::istringstream iss(s);
			return write(path, iss);
		}

		static inline bool exists(std::string path) {
			std::ifstream file(path);
			return file.good();
		}

		static inline void rename(std::string oldPath, std::string newPath) {
			if(::rename(oldPath.c_str(), newPath.c_str()) != 0) {
				debugmanager()->fatal("failed to rename `" + oldPath + "` to `" + newPath + '`');
			}
		}

		static inline void removefile(const std::string &path) {
			remove(path.c_str());
		}
	};
} // namespace polar::fs
