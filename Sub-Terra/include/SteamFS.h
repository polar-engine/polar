#pragma once

class SteamFS {
public:
	static bool Exists(std::string path) {
		return SteamRemoteStorage()->FileExists(path.data());
	}

	static size_t Size(std::string path) {
		return SteamRemoteStorage()->GetFileSize(path.data());
	}

	static std::string Read(std::string path) {
		size_t size = Size(path);
		char *sz = new char[size];
		SteamRemoteStorage()->FileRead(path.data(), sz, size);
		std::string s(sz, size);
		delete[] sz;
		return s;
	}

	static bool Write(std::string path, std::string s) {
		return SteamRemoteStorage()->FileWrite(path.data(), s.data(), s.size());
	}
};