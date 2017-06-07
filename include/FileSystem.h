#pragma once

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "debug.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#endif
#ifdef __APPLE__
#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

class FileSystem {
private:
	FileSystem() {}
public:
	static std::string DirOf(std::string path) {
		std::string::size_type pos = path.find_last_of("\\/");
		return path.substr(0, pos);
	}

	static std::string App() {
#if defined(_WIN32)
		char sz[MAX_PATH];
		GetModuleFileNameA(NULL, sz, MAX_PATH);
		return std::string(sz);
#else
		DebugManager()->Fatal("FileSystem::GetApp: not implemented");
		return "";
#endif
	}

	static std::string AppDir() {
#if defined(_WIN32)
		return DirOf(App());
#elif defined(__APPLE__)
		CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
		CFStringRef path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
		char sz[1024];
		CFStringGetCString(path, sz, 1024, CFStringGetSystemEncoding());
		CFRelease(url);
		CFRelease(path);
		return std::string(sz);
#else
		DebugManager()->Fatal("FileSystem::GetAppDir: not implemented");
#endif
	}

	static std::string SavedGamesDir() {
#if defined(_WIN32)
		char szPath[MAX_PATH];
		auto result = SHGetFolderPathA(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, szPath);
		if(FAILED(result)) { DebugManager()->Fatal("CSIDL_PERSONAL: failed to retrieve path"); }
		PathAppendA(szPath, "My Games");
		PathAppendA(szPath, "Freefall");
		return std::string(szPath);
#elif defined(__APPLE__)
		struct passwd *pwd = getpwuid(getuid());
		return std::string(pwd->pw_dir) + "/Documents/My Games/Freefall";
#else
		DebugManager()->Fatal("FileSystem::GetSavedGamesDir: not implemented");
		return "";
#endif
	}
	
	static std::string Read(std::string path) {
		std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
		if(file.fail()) { DebugManager()->Fatal(path + ": open"); }

		auto len = static_cast<std::string::size_type>(file.tellg());
		if(file.fail()) { DebugManager()->Fatal(path + ": tellg"); }

		file.seekg(0, std::ios::beg);
		if(file.fail()) { DebugManager()->Fatal(path + ": seekg"); }

		char *sz = new char[static_cast<unsigned int>(len + 1)];
		sz[len] = '\0';

		file.read(sz, static_cast<unsigned int>(len));
		if(file.fail()) { DebugManager()->Fatal(path + ": read"); }

		file.close();
		if(file.fail()) { DebugManager()->Fatal(path + ": close"); }

		std::string s(sz, len);
		delete[] sz;
		return s;
	}

	static bool Write(std::string path, std::istream &is) {
		CreateDir(DirOf(path));

		std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if(file.fail()) {
			DebugManager()->Error(path + ": open");
			return false;
		}

		file << is.rdbuf();
		if(file.fail()) {
			DebugManager()->Error(path + ": write");
			return false;
		}

		file.close();
		if(file.fail()) {
			DebugManager()->Error(path + ": close");
			return false;
		}

		return true;
	}

	static bool Write(std::string path, std::string s) {
		std::istringstream iss(s);
		return Write(path, iss);
	}

	static bool Exists(std::string path) {
		std::ifstream file(path);
		return file.good();
	}

	static uint64_t ModifiedTime(std::string path) {
#if defined(_WIN32)
		struct _stat st;
		if(_stat(path.c_str(), &st) != 0) { DebugManager()->Fatal(path + ": failed to stat"); }
#else
		struct stat st;
		if(stat(path.c_str(), &st) != 0) { DebugManager()->Fatal(path + ": failed to stat"); }
#endif
		return st.st_mtime;
	}

	static void Rename(std::string oldPath, std::string newPath) {
		if(rename(oldPath.c_str(), newPath.c_str()) != 0) { DebugManager()->Fatal("failed to rename `" + oldPath + "` to `" + newPath + '`'); }
	}

	static void RemoveFile(const std::string &path) {
		remove(path.c_str());
	}

	static std::vector<std::string> ListDir(std::string path) {
		std::vector<std::string> files;
#if defined(_WIN32)
		/* append "\*" to path and create wstring */
		path += "\\*";
		std::wstring wPath(path.begin(), path.end());

		WIN32_FIND_DATAW fdd;
		HANDLE handle = FindFirstFileW(wPath.c_str(), &fdd);
		if(handle == INVALID_HANDLE_VALUE) { DebugManager()->Fatal(path + ": failed to find first file"); }
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
			DebugManager()->Fatal("failed to find next file");
		}

		FindClose(handle);
#elif defined(__APPLE__)
		DIR *dp = opendir(path.c_str());
		if(dp == nullptr) { DebugManager()->Fatal(path + ": failed to open directory"); }

		struct dirent *ep;
		while((ep = readdir(dp))) {
			struct stat st;
			lstat(ep->d_name, &st);
			if(S_ISDIR(st.st_mode)) {
				files.emplace_back(ep->d_name);
			}
		}
		closedir(dp);
#else
		DebugManager()->Fatal("FileSystem::ListDir: not implemented");
#endif
		return files;
	}

	static void CreateDirImpl(std::string path) {
#if defined(_WIN32)
		std::wstring wPath(path.begin(), path.end());
		SetLastError(ERROR_SUCCESS);
		if(::CreateDirectoryW(wPath.c_str(), NULL) == 0) {
			DWORD dwError = GetLastError();
			if(dwError != ERROR_ALREADY_EXISTS) { DebugManager()->Fatal("failed to create directory ", path, " (error ", dwError, ')'); }
		}
#elif defined(__APPLE__)
		if(mkdir(path.c_str(), 0755) == -1 && errno != EEXIST) { DebugManager()->Fatal("failed to create directory ", path); }
#else
		DebugManager()->Fatal("FileSystem::CreateDirImpl: not implemented");
#endif
	}

	static void CreateDir(std::string path) {
		size_t pos = 0;
		do {
			pos = path.find_first_of("/\\", pos + 1);
			auto subpath = path.substr(0, pos);

			// work around Windows failing to create C:, E:, etc
			if(subpath[1] != ':') { CreateDirImpl(subpath); }
		} while(pos != path.npos);
	}
};
