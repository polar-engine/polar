#pragma once

#include <string>
#include <fstream>
#include <vector>
#include "debug.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

class FileSystem {
private:
	FileSystem() {}
public:
	static std::string FileRead(const std::string &name) {
		std::ifstream file(name, std::ios::in | std::ios::binary | std::ios::ate);
		if(file.fail()) { ENGINE_THROW(name + ": open"); }

		std::streamoff len = file.tellg();
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
	static void FileWrite(const std::string &name, std::string &data) {
		std::ofstream file(name, std::ios::out | std::ios::binary | std::ios::trunc);
		if(file.fail()) { ENGINE_THROW(name + ": open"); }

		file << data;
		if(file.fail()) { ENGINE_THROW(name + ": <<"); }

		file.close();
		if(file.fail()) { ENGINE_THROW(name + ": close"); }
	}

	static std::vector<std::string> DirList(std::string path) {
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
		return files;
#endif
	}

	static void DirCreate(const std::string &path) {
#ifdef _WIN32
		std::wstring wPath(path.begin(), path.end());
		if(::CreateDirectory(wPath.c_str(), NULL) == 0) {
			DWORD dwError = GetLastError();
			if(dwError != ERROR_ALREADY_EXISTS) { ENGINE_THROW(path + ": failed to create directory"); }
		}
#endif
	}
};
