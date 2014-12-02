#include <iostream>
#include <sstream>
#include <unordered_map>
#include <functional>

/* CF defines types Point and Component so it needs to be included early */
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "FileSystem.h"
#include "assets.h"
#include "debug.h"

int main(int argc, char **argv) {
	std::string path = argc >= 2 ? argv[1] : FileSystem::GetAppDir() + "/assets";
	std::string buildPath = argc >= 3 ? argv[2] : path + "/build";
	auto files = FileSystem::ListDir(path);

	std::unordered_map<std::string, std::function<Asset *(const std::string &)>> converters;
	converters["txt"] = [] (const std::string &data) {
		return new TextAsset(static_cast<uint32_t>(data.length()), data);
	};
	converters["gls"] = [] (const std::string &data) {
		std::vector<Shader> shaders;
		std::istringstream iss(data);
		std::string line;
		std::ostringstream oss;
		std::ostringstream header;
		ShaderType type = ShaderType::Invalid;

		for(int iLine = 1; std::getline(iss, line); ++iLine) {
			auto len = line.length();
			if(len > 0) {
				if(line[0] == '@') {
					std::stringstream msg;
					msg << iLine << ": missing shader directive";
					if(len < 2) { ENGINE_THROW(msg.str()); }
					auto directive = line.substr(1);
					if(directive == "vertex") {
						if(type != ShaderType::Invalid) { shaders.emplace_back(type, oss.str()); }
						type = ShaderType::Vertex;
						oss = std::ostringstream();
						oss << header.str();
					} else if(directive == "fragment") {
						if(type != ShaderType::Invalid) { shaders.emplace_back(type, oss.str()); }
						type = ShaderType::Fragment;
						oss = std::ostringstream();
						oss << header.str();
					} else { ENGINE_ERROR(iLine << ": unknown shader directive"); }
				} else {
					if(type == ShaderType::Invalid) { header << line << '\n'; } else { oss << line << '\n'; }
				}
			}
		}
		std::string rest = oss.str();
		if(!rest.empty()) { shaders.emplace_back(type, rest); }

		return new ShaderAsset(shaders);
	};

	for(auto &file : files) {
		auto pos = file.find_last_of('.');
		if(pos != file.npos && pos < file.length() - 1) {
			std::string ext = file.substr(pos + 1);
			std::string name = file.substr(0, pos);
			std::string data = FileSystem::ReadFile(path + '/' + file);
			auto converter = converters.find(ext);
			if(converter != converters.end()) {
				Asset *asset = converter->second(data);
				FileSystem::CreateDir(buildPath);
				FileSystem::CreateDir(buildPath + '/' + asset->type);
				FileSystem::WriteFile(buildPath + '/' + asset->type + '/' + name + ".asset", asset->Save());
				delete asset;
			} else {
				std::cerr << file << ": no appropriate Asset class found" << std::endl;
			}
		}
	}
	std::cout << "Press enter to continue" << std::endl;
	std::cin.ignore(1);
	return 0;
}
