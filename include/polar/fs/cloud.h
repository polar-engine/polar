#pragma once

namespace polar { namespace fs {
	class cloud {
	public:
		static bool exists(std::string path) {
			return SteamRemoteStorage()->FileExists(path.data());
		}

		static size_t size(std::string path) {
			return SteamRemoteStorage()->GetFileSize(path.data());
		}

		static std::string read(std::string path) {
			size_t siz = size(path);
			char *sz = new char[siz];
			SteamRemoteStorage()->FileRead(path.data(), sz, siz);
			std::string s(sz, siz);
			delete[] sz;
			return s;
		}

		static bool write(std::string path, std::string s) {
			return SteamRemoteStorage()->FileWrite(path.data(), s.data(), s.size());
		}
	};
} }
