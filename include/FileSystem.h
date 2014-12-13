#pragma once

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string>
#include <fstream>
#include <vector>
#include "debug.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#ifdef __APPLE__
#include <dirent.h>
#endif

class FileSystem {
private:
	FileSystem() {}
public:
	static std::string GetDirOf(const std::string &path) {
		std::string::size_type pos = path.find_last_of("\\/");
		return path.substr(0, pos);
	}
	static std::string GetDirOf(std::string &&path) { return GetDirOf(path); }

	static std::string GetApp() {
#ifdef _WIN32
		char sz[MAX_PATH];
		GetModuleFileNameA(NULL, sz, MAX_PATH);
		return std::string(sz);
#endif
#ifdef __APPLE__
#error "FileSystem::GetApp: not implemented"
#endif
	}

	static std::string GetAppDir() {
		return GetDirOf(GetApp());
		/*
#ifdef __APPLE__
		CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
		CFStringRef path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
		char sz[1024];
		CFStringGetCString(path, sz, 1024, CFStringGetSystemEncoding());
		CFRelease(url);
		CFRelease(path);
		return std::string(sz);
#endif
		*/
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
		CreateDir(GetDirOf(name));

		std::ofstream file(name, std::ios::out | std::ios::binary | std::ios::trunc);
		if(file.fail()) { ENGINE_THROW(name + ": open"); }

		file << data;
		if(file.fail()) { ENGINE_THROW(name + ": <<"); }

		file.close();
		if(file.fail()) { ENGINE_THROW(name + ": close"); }
	}
	static void WriteFile(const std::string &name, std::string &&data) { WriteFile(name, data); }

	static bool FileExists(const std::string &path) {
		std::ifstream file(path);
		return file.good();
	}

	static uint64_t GetModifiedTime(const std::string &path) {
#ifdef _WIN32
		struct _stat st;
		if(_stat(path.c_str(), &st) != 0) { ENGINE_THROW(path + ": failed to stat"); }
		return st.st_mtime;
#endif
#ifdef __APPLE__
#error "FileSystem::GetModifiedTime: not implemented"
#endif
	}

	static void Rename(const std::string &oldPath, const std::string &newPath) {
		rename(oldPath.c_str(), newPath.c_str());
	}

	static void RemoveFile(const std::string &path) {
		remove(path.c_str());
	}

	static std::vector<std::string> ListDir(std::string path) {
		std::vector<std::string> files;
#ifdef _WIN32
		/* append "\*" to path and create wstring */
		path += "\\*";
		std::wstring wPath(path.begin(), path.end());

		WIN32_FIND_DATAW fdd;
		HANDLE handle = FindFirstFileW(wPath.c_str(), &fdd);
		if(handle == INVALID_HANDLE_VALUE) { ENGINE_THROW(path + ": failed to find first file"); }
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
		if(::CreateDirectoryW(wPath.c_str(), NULL) == 0) {
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
	static void CreateDir(std::string &&path) { CreateDir(path); }
};
