#include "FileSystem.h"
#include "curl-config-win32.h"
#include "curl/curl.h"



int main(int argc, char **argv) {
	curl_global_init(CURL_GLOBAL_ALL);
	auto handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, "http://shockk.me/sub-terra/files/launcher-latest.zip");
	curl_easy_perform(handle);
	curl_easy_cleanup(handle);
	curl_global_cleanup();
	std::cin.ignore(1);
	return 0;
}
