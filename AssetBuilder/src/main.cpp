#include <iostream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include "FileSystem.h"
#include "assets.h"
#include "debug.h"

int main(int argc, char **argv) {
	std::string path = argc >= 2 ? argv[1] : "assets";
	std::string buildPath = argc >= 3 ? argv[2] : path + "/build";
	auto files = FileSystem::DirList(path);

	std::unordered_map<std::string, std::function<Asset *(const std::string &)>> converters;
	converters["txt"] = [] (const std::string &data) {
		return new TextAsset(data.length(), data);
	};
	converters["gls"] = [] (const std::string &data) {
		std::vector<Shader> shaders;

		std::istringstream iss(data);
		std::ostringstream oss;
		std::string s;
		while(std::getline(iss, s)) {
			oss << s;
		}

		const std::string delim = "##";
		std::string::size_type pos = 0;
		while((pos = data.find(delim, pos)) != data.npos) {
			pos += 2;
			if(data.substr(pos) == "end") { break; }
			std::string::size_type nextPos = data.find(delim, pos);
			if(nextPos == data.npos) { ENGINE_THROW("missing shader end of type"); }

			std::string typeString = data.substr(pos, nextPos - pos);

			ShaderType type;
			if(typeString == "vertex") {
				type = ShaderType::Vertex;
			} else if(typeString == "fragment") {
				type = ShaderType::Fragment;
			} else if(typeString == "end") {
				break;
			} else { ENGINE_THROW("invalid shader type"); }

			pos = nextPos + 2;
		}
		return new ShaderAsset(std::vector<Shader>());
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
				FileSystem::DirCreate(buildPath);
				FileSystem::DirCreate(buildPath + '/' + asset->type);
				FileSystem::FileWrite(buildPath + '/' + asset->type + '/' + name + ".asset", asset->Save());
				delete asset;
			} else {
				std::cerr << file << ": no appropriate Asset class found" << std::endl;
			}
		}
	}
	std::cin.ignore(1);
	return 0;
}
