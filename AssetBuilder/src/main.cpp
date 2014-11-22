#include <iostream>
#include <unordered_map>
#include <functional>
#include "FileSystem.h"
#include "assets.h"

int main(int argc, char **argv) {
	std::string path = argc >= 2 ? argv[1] : "assets";
	auto files = FileSystem::DirList(path);

	std::unordered_map<std::string, std::function<Asset *(const std::string &)>> converters;
	converters["txt"] = [] (const std::string &data) {
		return new TextAsset(data.length(), data);
	};

	for(auto &file : files) {
		auto pos = file.find_last_of('.');
		if(pos != file.npos && pos < file.length() - 1) {
			std::string ext = file.substr(pos + 1);
			std::string name = file.substr(0, pos);
			std::string data = FileSystem::FileRead(path + '/' + file);
			auto converter = converters.find(ext);
			if(converter != converters.end()) {
				Asset *asset = converter->second(data);
				FileSystem::DirCreate(path + "/build");
				FileSystem::DirCreate(path + "/build/" + asset->type);
				FileSystem::FileWrite(path + "/build/" + asset->type + '/' + name + ".asset", asset->Save());
				delete asset;
			} else {
				std::cerr << file << ": no appropriate Asset class found" << std::endl;
				std::cin.ignore(1);
				return 1;
			}
		}
	}
	std::cin.ignore(1);
	return 0;
}
