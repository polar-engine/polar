#include <string>
#include "FileSystem.h"
#include "curl-config-win32.h"
#include "curl/curl.h"
#include "zip/zip.h"

#define INFO(msg) (std::cout << "[INFO] " << (msg) << std::endl)
#define INFOS(msg) (std::cout << "[INFO] " << msg << std::endl)
#define FATAL(msg) (std::cerr << "[FATAL] " << (msg) << std::endl, throw std::runtime_error(msg))
#ifdef _DEBUG
#define CONTINUE (std::cin.ignore(1))
#else
#define CONTINUE
#endif

size_t cbWrite(void *buffer, size_t size, size_t nmemb, void *userp) {
	size_t nBytes = size * nmemb;
	reinterpret_cast<std::string *>(userp)->append(reinterpret_cast<char *>(buffer), nBytes);
	return nBytes;
}

int main(int argc, char **argv) {
	std::string zipBuffer;

	curl_global_init(CURL_GLOBAL_ALL);

	const std::string archiveURL = "http://shockk.me/sub-terra/files/sub-terra-latest.zip";
	INFO("downloading latest archive from server...");

	auto handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, archiveURL.c_str());
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, cbWrite);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &zipBuffer);
	curl_easy_perform(handle);
	curl_easy_cleanup(handle);

	INFO("done");

	curl_global_cleanup();

	const std::string launcherDir = FileSystem::GetAppDir();
	const std::string appDir = launcherDir + "/sub-terra";
	const std::string archiveName = "sub-terra-latest.zip";
	const std::string archivePath = launcherDir + '/' + archiveName;

	INFO("writing archive to launcher dir...");
	FileSystem::WriteFile(archivePath, zipBuffer);
	INFO("done");

	FileSystem::CreateDir(appDir);

	struct zip *archive = zip_open(archivePath.c_str(), ZIP_CHECKCONS, NULL);
	if(archive == NULL) { FATAL("failed to open archive `" + archiveName + '`'); }
	INFO("opened archive");

	for(int i = 0; i < zip_get_num_files(archive); ++i) {
		const char *szName = zip_get_name(archive, i, ZIP_FL_ENC_GUESS);
		if(szName == NULL) { FATAL("failed to get file name in `" + archiveName + "` at index " + std::to_string(i)); }
		std::string name(szName);

		if(name.back() == '/') { /* directory */
			FileSystem::CreateDir(appDir + '/' + name);
			INFOS("D: " << name);
		} else {
			struct zip_stat st;
			if(zip_stat_index(archive, i, 0, &st) != 0) { FATAL("failed to stat `" + name + "` in `" + archiveName + '`'); }

			struct zip_file *fh = zip_fopen_index(archive, i, 0);
			if(fh == NULL) { FATAL("failed to open `" + name + "` in `" + archiveName + '`'); }

			char *buf = new char[st.size];

			zip_int64_t len = zip_fread(fh, buf, st.size);
			if(len == -1) { FATAL("failed to read `" + name + "` in `" + archiveName + '`'); }

			FileSystem::WriteFile(appDir + '/' + name, std::string(buf, len));

			delete buf;

			INFOS("F: " << name);
		}
	}

	if(zip_close(archive) != 0) { FATAL("failed to close archive `" + archiveName + '`'); }
	INFO("closed archive");

	CONTINUE;
	return 0;
}
