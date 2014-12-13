#pragma once

#include <string>
#include <fstream>
#include <vector>
#include "debug.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#ifdef __APPLE__
#include <sys/stat.h>
#include <dirent.h>
#endif

class FileSystem {
private:
	FileSystem() {}
public:
	static std::string GetAppDir() {
#ifdef _WIN32
		char sz[MAX_PATH];
		GetModuleFileNameA(NULL, sz, MAX_PATH);
		std::string s(sz);
		std::string::size_type pos = s.find_last_of("\\/");
		return s.substr(0, pos);
#endif
#ifdef __APPLE__
		CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
		CFStringRef path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
		char sz[1024];
		CFStringGetCString(path, sz, 1024, CFStringGetSystemEncoding());
		CFRelease(url);
		CFRelease(path);
		return std::string(sz);
#endif
	}
	
	static std::string ReadFile(const std::string &name) {
		std::ifstream file(name, std::ios::in | std::ios::binary | std::ios::ate);
		if(file.fail()) { ENGINE_THROW(name + ": open"); }

		auto len = static_cast<std::string::size_type>(file.tellg());
		if(file.fail()) { ENGINE_THROW(name + ": tellg"); }

		file.seekg(0, std::ios::beg);
		if(file.fail()) { ENGINE_THROW(name + ": seekg"); }

		char *sz = new char[static_cast<unsigned int>(len + 1)];
		sz[len] = '\0';

		file.read(sz, static_cast<unsigned int>(len));
		if(file.fail()) { ENGINE_THROW(name + ": read"); }

		file.close();
		if(file.fail()) { ENGINE_THROW(name + ": close"); }

		std::string s(sz, len);
		return s;
	}

	static void WriteFile(const std::string &name, std::string &data) {
		std::ofstream file(name, std::ios::out | std::ios::binary | std::ios::trunc);
		if(file.fail()) { ENGINE_THROW(name + ": open"); }

		file << data;
		if(file.fail()) { ENGINE_THROW(name + ": <<"); }

		file.close();
		if(file.fail()) { ENGINE_THROW(name + ": close"); }
	}

	static void WriteFile(const std::string &name, std::string &&data) {
		WriteFile(name, data);
	}

	static std::vector<std::string> ListDir(std::string path) {
		std::vector<std::string> files;
#ifdef _WIN32
		/* append "\*" to path and create wstring */
		path += "\\*";
		std::wstring wPath(path.begin(), path.end());

		WIN32_FIND_DATA fdd;
		HANDLE handle = FindFirstFile(wPath.c_str(), &fdd);
		if(handle == INVALID_HANDLE_VALUE) { ENGINE_THROW(path + ": failed to find first file"); }
		do {
			if(!(fdd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				std::wstring wFile = fdd.cFileName;
				files.emplace_back(wFile.begin(), wFile.end());
			}
		} while(FindNextFile(handle, &fdd));

		/* check for errors */
		DWORD dwError = GetLastError();
		if(dwError != ERROR_NO_MORE_FILES) {
			FindClose(handle);
			ENGINE_THROW("failed to find next file");
		}

		FindClose(handle);
#endif
#ifdef __APPLE__
		DIR *dp = opendir(path.c_str());
		if(dp == nullptr) { ENGINE_THROW(path + ": failed to open directory"); }

		struct dirent *ep;
		while((ep = readdir(dp))) {
			struct stat st;
			lstat(ep->d_name, &st);
			if(S_ISDIR(st.st_mode)) {
				files.emplace_back(ep->d_name);
			}
		}
		closedir(dp);
#endif
		return files;
	}

	static void CreateDirImpl(const std::string &path) {
#ifdef _WIN32
		std::wstring wPath(path.begin(), path.end());
		if(::CreateDirectory(wPath.c_str(), NULL) == 0) {
			DWORD dwError = GetLastError();
			if(dwError != ERROR_ALREADY_EXISTS) { ENGINE_THROW(path + ": failed to create directory"); }
		}
#endif
#ifdef __APPLE__
		if(mkdir(path.c_str(), 0755) == -1 && errno != EEXIST) { ENGINE_THROW(path + ": failed to create directory"); }
#endif
	}

	static void CreateDir(const std::string &path) {
		size_t pos = 0;
		do {
			pos = path.find_first_of("/\\", pos + 1);
			CreateDirImpl(path.substr(0, pos));
		} while(pos != path.npos);
	}
};
