#pragma once

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <polar/core/debugmanager.h>
#include <polar/util/debug.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#elif defined(__linux__)
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/types.h>
#elif defined(__APPLE__)
#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace polar { namespace fs {
	class local {
	private:
		local() {}
	public:
		static std::string dir_of(std::string path) {
			std::string::size_type pos = path.find_last_of("\\/");
			return path.substr(0, pos);
		}

		static std::string app() {
	#if defined(_WIN32)
			char sz[MAX_PATH];
			GetModuleFileNameA(NULL, sz, MAX_PATH);
			return std::string(sz);
	#elif defined(__linux__)
			char sz[1024];
			readlink("/proc/self/exe", sz, sizeof(sz) - 1);
			return std::string(sz);
	#else
			debugmanager()->fatal("polar::fs::local::app: not implemented");
			return "";
	#endif
		}

		static std::string appdir() {
	#if defined(_WIN32) || defined(__linux__)
			return dir_of(app());
	#elif defined(__APPLE__)
			CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
			CFStringRef path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
			char sz[1024];
			CFStringGetCString(path, sz, 1024, CFStringGetSystemEncoding());
			CFRelease(url);
			CFRelease(path);
			return std::string(sz);
	#else
			debugmanager()->fatal("polar::fs::local::appdir: not implemented");
			return "";
	#endif
		}

		static std::string savedgamesdir() {
	#if defined(_WIN32)
			char szPath[MAX_PATH];
			auto result = SHGetFolderPathA(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, szPath);
			if(FAILED(result)) { debugmanager()->fatal("CSIDL_PERSONAL: failed to retrieve path"); }
			PathAppendA(szPath, "My Games");
			PathAppendA(szPath, "Freefall");
			return std::string(szPath);
	#elif defined(__linux__)
			struct passwd *pwd = getpwuid(getuid());
			return std::string(pwd->pw_dir) + "/mygames/Freefall";
	#elif defined(__APPLE__)
			struct passwd *pwd = getpwuid(getuid());
			return std::string(pwd->pw_dir) + "/Documents/My Games/Freefall";
	#else
			debugmanager()->fatal("polar::fs::local::savedgamesdir: not implemented");
			return "";
	#endif
		}

		static std::string read(std::string path, size_t offset = 0, size_t len = 0,  bool *eof = nullptr) {
			std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
			if(file.fail()) { debugmanager()->fatal(path + ": open"); }

			size_t filelen = file.tellg();
			if(file.fail()) { debugmanager()->fatal(path + ": tellg"); }

			if(len == 0 || len > filelen - offset) {
				len = filelen - offset;
				if(eof) {
					*eof = true;
				}
			}

			file.seekg(offset, std::ios::beg);
			if(file.fail()) { debugmanager()->fatal(path + ": seekg"); }

			char *sz = new char[static_cast<unsigned int>(len + 1)];
			sz[len] = '\0';

			file.read(sz, static_cast<unsigned int>(len));
			if(file.fail()) { debugmanager()->fatal(path + ": read"); }

			file.close();
			if(file.fail()) { debugmanager()->fatal(path + ": close"); }

			std::string s(sz, len);
			delete[] sz;
			return s;
		}

		static bool write(std::string path, std::istream &is) {
			createdir(dir_of(path));

			std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
			if(file.fail()) {
				debugmanager()->error(path + ": open");
				return false;
			}

			file << is.rdbuf();
			if(file.fail()) {
				debugmanager()->error(path + ": write");
				return false;
			}

			file.close();
			if(file.fail()) {
				debugmanager()->error(path + ": close");
				return false;
			}

			return true;
		}

		static bool write(std::string path, std::string s) {
			std::istringstream iss(s);
			return write(path, iss);
		}

		static bool exists(std::string path) {
			std::ifstream file(path);
			return file.good();
		}

		static uint64_t modifiedtime(std::string path) {
	#if defined(_WIN32)
			struct _stat st;
			if(_stat(path.c_str(), &st) != 0) { debugmanager()->fatal(path + ": failed to stat"); }
	#else
			struct stat st;
			if(stat(path.c_str(), &st) != 0) { debugmanager()->fatal(path + ": failed to stat"); }
	#endif
			return st.st_mtime;
		}

		static void rename(std::string oldPath, std::string newPath) {
			if(::rename(oldPath.c_str(), newPath.c_str()) != 0) { debugmanager()->fatal("failed to rename `" + oldPath + "` to `" + newPath + '`'); }
		}

		static void removefile(const std::string &path) {
			remove(path.c_str());
		}

		static std::vector<std::string> listdir(std::string path) {
			std::vector<std::string> files;
	#if defined(_WIN32)
			/* append "\*" to path and create wstring */
			path += "\\*";
			std::wstring wPath(path.begin(), path.end());

			WIN32_FIND_DATAW fdd;
			HANDLE handle = FindFirstFileW(wPath.c_str(), &fdd);
			if(handle == INVALID_HANDLE_VALUE) { debugmanager()->fatal(path + ": failed to find first file"); }
			do {
				if(!(fdd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					std::wstring wFile = fdd.cFileName;
					files.emplace_back(wFile.begin(), wFile.end());
				}
			} while(FindNextFileW(handle, &fdd));

			/* check for errors */
			DWORD dwError = GetLastError();
			if(dwError != ERROR_NO_MORE_FILES) {
				FindClose(handle);
				debugmanager()->fatal("failed to find next file");
			}

			FindClose(handle);
	#elif defined(__APPLE__) || defined(__linux__)
			DIR *dp = opendir(path.c_str());
			if(dp == nullptr) { debugmanager()->fatal(path, ": failed to open directory"); }

			struct dirent *ep;
			while((ep = readdir(dp))) {
				struct stat st;
				int result = fstatat(dirfd(dp), ep->d_name, &st, AT_SYMLINK_NOFOLLOW);
				if(result < 0) {
					debugmanager()->fatal(path, ": fstatat failed on ", ep->d_name);
				}

				if(!S_ISDIR(st.st_mode)) {
					debugmanager()->warning(ep->d_name);
					files.emplace_back(ep->d_name);
				}
			}
			closedir(dp);
	#else
			debugmanager()->fatal("FileSystem::ListDir: not implemented");
	#endif
			return files;
		}

		static void createdir_impl(std::string path) {
	#if defined(_WIN32)
			std::wstring wPath(path.begin(), path.end());
			SetLastError(ERROR_SUCCESS);
			if(::CreateDirectoryW(wPath.c_str(), NULL) == 0) {
				DWORD dwError = GetLastError();
				if(dwError != ERROR_ALREADY_EXISTS) { debugmanager()->fatal("failed to create directory ", path, " (error ", dwError, ')'); }
			}
	#elif defined(__APPLE__) || defined(__linux__)
			if(mkdir(path.c_str(), 0755) == -1 && errno != EEXIST) { debugmanager()->fatal("failed to create directory ", path); }
	#else
			debugmanager()->fatal("FileSystem::CreateDirImpl: not implemented");
	#endif
		}

		static void createdir(std::string path) {
			size_t pos = 0;
			do {
				pos = path.find_first_of("/\\", pos + 1);
				auto subpath = path.substr(0, pos);

				// work around Windows failing to create C:, E:, etc
				if(subpath[1] != ':' || subpath.size() > 3) {
					debugmanager()->verbose("creating directory ", path);
					createdir_impl(subpath);
				}
			} while(pos != path.npos);
		}
	};
} }
