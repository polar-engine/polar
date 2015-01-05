#include <iostream>
#include <sstream>
#include <unordered_map>
#include <functional>

/* CF defines types Point and Component so it needs to be included early */
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "debug.h"
#include "getline.h"
#include "FileSystem.h"
#include "assets.h"

int main(int argc, char **argv) {
	std::string path = argc >= 2 ? argv[1] : FileSystem::GetAppDir() + "/assets";
	std::string buildPath = argc >= 3 ? argv[2] : path + "/build";
	auto files = FileSystem::ListDir(path);

	std::unordered_map<std::string, std::function<Asset *(const std::string &)>> converters;
	converters["txt"] = [] (const std::string &data) {
		return new TextAsset(static_cast<uint32_t>(data.length()), data);
	};
	converters["gls"] = [] (const std::string &data) {
		ShaderAsset *asset = new ShaderAsset(std::vector<Shader>());

		std::ostringstream header;

		std::vector<std::tuple<std::string, std::string>> uniforms;
		std::vector<std::tuple<std::string, std::string>> attribs;
		std::vector<std::tuple<std::string, std::string, std::string>> varyings;
		std::vector<std::tuple<std::string, std::string>> outs;

		std::istringstream iss(data);
		std::string line;
		for(int iLine = 1; getline(iss, line); ++iLine) {
			auto len = line.length();
			if(len > 0) {
				if(line[0] == '@') {
					std::istringstream ls(line.substr(1));

					std::string directive;
					std::getline(ls, directive, ' ');
					if(!ls.good() && directive.empty()) { ENGINE_ERROR(iLine << ": missing directive"); }

					std::vector<std::string> args;
					std::string tmp;
					while(ls.good()) {
						std::getline(ls, tmp, ' ');
						if(!tmp.empty()) { args.emplace_back(tmp); }
					}

					if(directive == "shader") {
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing shader type"); }

						if(args[0] == "vertex") {
							asset->shaders.emplace_back(ShaderType::Vertex, header.str());
						} else if(args[0] == "fragment") {
							asset->shaders.emplace_back(ShaderType::Fragment, header.str());
						} else { ENGINE_ERROR(iLine << ": unknown shader type `" << args[0] << '`'); }
					} else if(directive == "uniform") {
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing uniform type"); }
						if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing uniform name"); }
						uniforms.emplace_back(args[0], args[1]);
					} else if(directive == "attrib") {
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing attribute type"); }
						if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing attribute name"); }
						attribs.emplace_back(args[0], args[1]);
					} else if(directive == "varying") {
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing varying interpolation method"); }
						if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing varying type"); }
						if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing varying name"); }
						varyings.emplace_back(args[0], args[1], args[2]);
					} else if(directive == "in") {
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing program input type"); }
						if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing program input name"); }

						if(args[0] == "color") {
							ENGINE_ERROR(iLine << ": color program input not implemented yet");
						} else if(args[0] == "depth") {
							ENGINE_ERROR(iLine << ": depth program input not implemented yet");
						} else { ENGINE_ERROR(iLine << ": unknown program output type"); }

						uniforms.emplace_back("sampler2D", args[1]);
					} else if(directive == "out") {
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing program output type"); }

						if(args[0] == "color") {
							if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing program output name"); }
							outs.emplace_back("vec4", args[1]);
						} else if(args[0] == "depth") {
							ENGINE_ERROR(iLine << ": depth program output not implemented yet");
						} else { ENGINE_ERROR(iLine << ": unknown program output type"); }
					} else { ENGINE_ERROR(iLine << ": unknown directive `" << directive << '`'); }
				} else {
					if(asset->shaders.empty()) { header << line << '\n'; } else { asset->shaders.back().source += line + '\n'; }
				}
			}
		}

		for(auto &shader : asset->shaders) {
			std::string prepend;
			prepend += "#version 150\n";
			prepend += "precision highp float;\n";
			for(auto &uniform : uniforms) {
				prepend += "uniform " + std::get<0>(uniform) + ' ' + std::get<1>(uniform) + ";\n";
			}
			if(shader.type == ShaderType::Vertex) {
				prepend += "#extension GL_ARB_explicit_attrib_location: enable\n";
				int attribLoc = 0;
				for(auto &attrib : attribs) {
					prepend += "layout(location=" + std::to_string(attribLoc++) + ") in " + std::get<0>(attrib) +' ' + std::get<1>(attrib) +";\n";
				}
				for(auto &varying : varyings) {
					prepend += std::get<0>(varying) + " out " + std::get<1>(varying) + ' ' + std::get<2>(varying) + ";\n";
				}
			} else if(shader.type == ShaderType::Fragment) {
				for(auto &varying : varyings) {
					prepend += std::get<0>(varying) + " in " + std::get<1>(varying) + ' ' + std::get<2>(varying) + ";\n";
				}
				for(auto &out : outs) {
					prepend += "out " + std::get<0>(out) + ' ' + std::get<1>(out) + ";\n";
				}
			}
			shader.source = prepend + shader.source;
		}

		return asset;
	};

	for(auto &file : files) {
		INFOS("processing `" << file << '`');
		auto pos = file.find_last_of('.');
		if(pos != file.npos && pos < file.length() - 1) {
			std::string ext = file.substr(pos + 1);
			auto converter = converters.find(ext);
			if(converter != converters.end()) {
				INFOS("found converter for `" << ext << '`');

				std::string data = FileSystem::ReadFile(path + '/' + file);
				Asset *asset = converter->second(data);
				if(asset == nullptr) {
					ENGINE_ERROR(file << ": converter function returned a null pointer");
					continue;
				}

				std::string name = file.substr(0, pos);
				FileSystem::CreateDir(buildPath + '/' + asset->type);
				FileSystem::WriteFile(buildPath + '/' + asset->type + '/' + name + ".asset", asset->Save());
				delete asset;
			} else { ENGINE_ERROR(file << ": no appropriate converter found"); }
		}
	}
	ENGINE_CONTINUE;
	return 0;
}
