#pragma once

#include <string>
#include <fstream>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

class FileSystem {
private:
	FileSystem() {}
public:
	static std::string Load(const std::string &name) {
		std::ifstream file(name, std::ios::in | std::ios::binary | std::ios::ate);
		if(file) {
			std::streamoff len = file.tellg();
			file.seekg(0, std::ios::beg);
			char *sz = new char[static_cast<unsigned int>(len + 1)];
			sz[len] = '\0';
			file.read(sz, static_cast<unsigned int>(len));
			return std::string(sz);
		}
	}
	static std::vector<std::string> Contents(std::string path) {
		std::vector<std::string> files;
#ifdef _WIN32
		/* append "\*" to path and create wstring */
		path += "\\*";
		std::wstring wPath(path.begin(), path.end());

		WIN32_FIND_DATA fdd;
		HANDLE handle = FindFirstFile(wPath.c_str(), &fdd);
		if(handle == INVALID_HANDLE_VALUE) { throw std::runtime_error("error finding first file"); }
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
			throw std::runtime_error("error finding next file");
		}

		FindClose(handle);
		return files;
#endif
	}
};
