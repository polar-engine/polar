#include <string>
#include <sstream>
#include "FileSystem.h"
#include "curl-config-win32.h"
#include "curl/curl.h"
#include "zip/zip.h"

#ifdef _WIN32
#include "Process.h"
#define execl _execl
#endif

#define INFO(msg) (std::cout << "[INFO] " << (msg) << std::endl)
#define INFOS(msg) (std::cout << "[INFO] " << msg << std::endl)
#define FATAL(msg) (std::cerr << "[FATAL] " << (msg) << std::endl, throw std::runtime_error(msg))
#ifdef _DEBUG
#define CONTINUE (std::cout << "Press enter to continue.", std::cin.ignore(1))
#else
#define CONTINUE
#endif

size_t cbWrite(void *buffer, size_t size, size_t nmemb, void *userp) {
	size_t nBytes = size * nmemb;
	reinterpret_cast<std::string *>(userp)->append(reinterpret_cast<char *>(buffer), nBytes);
	return nBytes;
}

std::string GetRemote(const char *url) {
	std::string data;
	auto handle = curl_easy_init();
	if(handle == NULL) { FATAL("failed to init cURL easy handle"); }
	if(curl_easy_setopt(handle, CURLOPT_URL, url) != CURLE_OK) { FATAL("failed to set CURLOPT_URL"); }
	if(curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, cbWrite) != CURLE_OK) { FATAL("failed to set CURLOPT_WRITEFUNCTION"); }
	if(curl_easy_setopt(handle, CURLOPT_WRITEDATA, &data) != CURLE_OK) { FATAL("failed to set CURLOPT_WRITEDATA"); }
	if(curl_easy_setopt(handle, CURLOPT_FILETIME, 1) != CURLE_OK) { FATAL("failed to set CURLOPT_FILETIME"); }
	if(curl_easy_perform(handle) != CURLE_OK) { FATAL("failed to perform cURL request"); }
	curl_easy_cleanup(handle);
	return data;
}
std::string GetRemote(const std::string &url) { return GetRemote(url.c_str()); }
std::string GetRemote(std::string &&url) { return GetRemote(url.c_str()); }

void UpdateLauncher() {

	INFO("checking for newer version of launcher");

	const std::string versionPath = FileSystem::GetAppDir() + "/version";
	auto localVersion = FileSystem::FileExists(versionPath) ? FileSystem::ReadFile(versionPath) : "";

	auto remoteVersion = GetRemote("http://shockk.me/sub-terra/files/launcher-latest");
	std::istringstream iss(remoteVersion);
	std::getline(iss, remoteVersion);

	if(localVersion != remoteVersion) {
		INFO("newer version of launcher detected");
		INFOS("downloading launcher v" << remoteVersion);

		auto launcher = GetRemote("http://shockk.me/sub-terra/files/launcher-" + remoteVersion + ".exe");

		INFO("writing new launcher to disk");

		const std::string appPath = FileSystem::GetApp();
		const std::string old = appPath + ".old";
		if(FileSystem::FileExists(old)) { FileSystem::RemoveFile(old); }
		FileSystem::Rename(appPath, appPath + ".old");
		auto ssLauncher = std::stringstream(launcher);
		auto ssRemoteVersion = std::stringstream(remoteVersion);
		FileSystem::WriteFile(appPath, ssLauncher);
		FileSystem::WriteFile(versionPath, ssRemoteVersion);

		INFO("restarting launcher");

		curl_global_cleanup();

		auto sz = appPath.c_str();
		execl(sz, sz, 0);
	}
}

void UpdateSubTerra() {
	const std::string appDir = FileSystem::GetAppDir();
	const std::string subTerraDir = appDir + "/sub-terra";
	const std::string archiveName = "sub-terra-latest.zip";
	const std::string archivePath = appDir + '/' + archiveName;

	INFO("checking for newer version of Sub-Terra");

	const std::string versionPath = appDir + "/sub-terra-version";
	auto localVersion = FileSystem::FileExists(versionPath) ? FileSystem::ReadFile(versionPath) : "";

	auto remoteVersion = GetRemote("http://shockk.me/sub-terra/files/sub-terra-latest");
	std::istringstream iss(remoteVersion);
	std::getline(iss, remoteVersion);

	if(localVersion != remoteVersion) {
		INFO("newer version of Sub-Terra detected");
		INFOS("downloading Sub-Terra v" << remoteVersion);

		auto archiveData = GetRemote("http://shockk.me/sub-terra/files/sub-terra-" + remoteVersion + ".zip");

		INFO("writing archive to disk");
		auto ssArchiveData = std::stringstream(archiveData);
		auto ssRemoteVersion = std::stringstream(remoteVersion);
		FileSystem::WriteFile(archivePath, ssArchiveData);
		FileSystem::WriteFile(versionPath, ssRemoteVersion);

		FileSystem::CreateDir(subTerraDir);

		struct zip *archive = zip_open(archivePath.c_str(), ZIP_CHECKCONS, NULL);
		if(archive == NULL) { FATAL("failed to open archive `" + archiveName + '`'); }
		INFO("opened archive");

		for(int i = 0; i < zip_get_num_files(archive); ++i) {
			const char *szName = zip_get_name(archive, i, ZIP_FL_ENC_GUESS);
			if(szName == NULL) { FATAL("failed to get file name in `" + archiveName + "` at index " + std::to_string(i)); }
			std::string name(szName);

			if(name.back() == '/') { /* directory */
				FileSystem::CreateDir(subTerraDir + '/' + name);
				INFOS("D: " << name);
			} else {
				struct zip_stat st;
				if(zip_stat_index(archive, i, 0, &st) != 0) { FATAL("failed to stat `" + name + "` in `" + archiveName + '`'); }

				struct zip_file *fh = zip_fopen_index(archive, i, 0);
				if(fh == NULL) { FATAL("failed to open `" + name + "` in `" + archiveName + '`'); }

				char *buf = new char[st.size];

				zip_int64_t len = zip_fread(fh, buf, st.size);
				if(len == -1) { FATAL("failed to read `" + name + "` in `" + archiveName + '`'); }

				auto ssFile = std::stringstream(std::string(buf, len));
				FileSystem::WriteFile(subTerraDir + '/' + name, ssFile);

				delete[] buf;

				INFOS("F: " << name);
			}
		}

		if(zip_close(archive) != 0) { FATAL("failed to close archive `" + archiveName + '`'); }
		INFO("closed archive");
	}
}

int main(int argc, char **argv) {
	if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) { FATAL("failed to init cURL"); }

	UpdateLauncher();
	UpdateSubTerra();

	curl_global_cleanup();

	const std::string path = FileSystem::GetAppDir() + "/sub-terra/Sub-Terra.exe";
	auto sz = path.c_str();
	execl(sz, sz, 0);

	return 0;
}
