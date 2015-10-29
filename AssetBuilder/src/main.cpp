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
#include "endian.h"
#include "FileSystem.h"
#include "TextAsset.h"
#include "ImageAsset.h"
#include "AudioAsset.h"
#include "ShaderProgramAsset.h"

int main(int argc, char **argv) {
	std::string path = argc >= 2 ? argv[1] : FileSystem::GetAppDir() + "/assets";
	std::string buildPath = argc >= 3 ? argv[2] : path + "/build";
	auto files = FileSystem::ListDir(path);

	std::unordered_map < std::string, std::function<std::string(const std::string &, Serializer &)>> converters;
	converters["txt"] = [] (const std::string &data, Serializer &s) {
		s << data;
		return AssetName<std::string>();
	};
	converters["png"] = [] (const std::string &data, Serializer &s) {
		ImageAsset asset;
		std::istringstream iss(data);

		std::string header(8, ' ');
		iss.read(&header[0], 8);
		if(header != "\x89" "PNG" "\x0D\x0A" "\x1A" "\xA") { ENGINE_THROW("invalid PNG signature"); }

		size_t numChunks;
		bool atEnd = false;

		for(numChunks = 0; !atEnd; ++numChunks) {
			uint32_t dataSize;
			iss.read(reinterpret_cast<char *>(&dataSize), sizeof(dataSize));
			dataSize = swapbe(dataSize);
			if(dataSize > (1u << 31)) { ENGINE_THROW("chunk data size exceeds 2^31 bytes"); }

			std::string chunkType(4, ' ');
			iss.read(&chunkType[0], 4);

			if(numChunks == 0) {
				if(chunkType == "IHDR") {
					iss.read(reinterpret_cast<char *>(&asset.width), sizeof(asset.width));
					asset.width = swapbe(asset.width);
					if(asset.width == 0) { ENGINE_THROW("image width cannot be 0"); }
					if(asset.width > (1u << 31)) { ENGINE_THROW("image width exceeds 2^31 bytes"); }

					iss.read(reinterpret_cast<char *>(&asset.height), sizeof(asset.height));
					asset.height = swapbe(asset.height);
					if(asset.height == 0) { ENGINE_THROW("image height cannot be 0"); }
					if(asset.height > (1u << 31)) { ENGINE_THROW("image height exceeds 2^31 bytes"); }

					uint8_t bitDepth;
					iss.read(reinterpret_cast<char *>(&bitDepth), sizeof(bitDepth));

					uint8_t colorType;
					iss.read(reinterpret_cast<char *>(&colorType), sizeof(colorType));
					if(!(colorType & 0b110)) { ENGINE_THROW("color type must be color & alpha"); }

					uint8_t compressionMethod;
					iss.read(reinterpret_cast<char *>(&compressionMethod), sizeof(compressionMethod));
					if(compressionMethod != 0) { ENGINE_THROW("compression method must be 0"); }

					uint8_t filterMethod;
					iss.read(reinterpret_cast<char *>(&filterMethod), sizeof(filterMethod));
					if(filterMethod != 0) { ENGINE_THROW("filter method must be 0"); }

					uint8_t interlaceMethod;
					iss.read(reinterpret_cast<char *>(&interlaceMethod), sizeof(interlaceMethod));
					if(interlaceMethod != 0 && interlaceMethod != 1) { ENGINE_THROW("interlace method must be 0 or 1"); }

					iss.ignore(4);
				} else { ENGINE_THROW("first chunk must be IHDR"); }
			} else {
				if(chunkType == "IDAT") {
					INFO("found image data chunk");
					iss.ignore(dataSize);
					iss.ignore(4);
				} else if(chunkType == "IEND") {
					INFO("found image end chunk");
					atEnd = true;
				} else if(chunkType == "bKGD") {
					iss.ignore(dataSize);
					iss.ignore(4);
				} else if(chunkType == "pHYs") {
					iss.ignore(dataSize);
					iss.ignore(4);
				} else if(chunkType == "tIME") {
					iss.ignore(dataSize);
					iss.ignore(4);
				} else if(chunkType == "iTXt") {
					iss.ignore(dataSize);
					iss.ignore(4);
				} else {
					std::stringstream ss;
					ss << "unrecognized chunk type `" << chunkType << '`';
					ENGINE_THROW(ss.str());
				}
			}
		}

		s << asset;
		return AssetName<ImageAsset>();
	};
	converters["wav"] = [] (const std::string &data, Serializer &s) {
		AudioAsset asset;
		std::istringstream iss(data);

		std::string riffHeader(4, ' ');
		iss.read(&riffHeader[0], 4);
		if(riffHeader != "RIFF") { ENGINE_THROW("missing riff header"); }

		uint32_t riffSize;
		iss.read(reinterpret_cast<char *>(&riffSize), sizeof(riffSize));
		riffSize = swaple(riffSize);

		uint32_t riffSizeAccum = 0;

		if(riffSize - riffSizeAccum < 4) { ENGINE_THROW("wave header does not fit into riff size"); }
		std::string waveHeader(4, ' ');
		iss.read(&waveHeader[0], 4);
		if(waveHeader != "WAVE") { ENGINE_THROW("missing wave header"); }
		riffSizeAccum += 4;

		while(riffSize - riffSizeAccum > 0) {
			if(riffSize - riffSizeAccum < 8) { ENGINE_THROW("chunk header and size do not fit into riff size"); }

			std::string chunkHeader(4, ' ');
			iss.read(&chunkHeader[0], 4);
			riffSizeAccum += 4;

			uint32_t chunkSize;
			iss.read(reinterpret_cast<char *>(&chunkSize), sizeof(chunkSize));
			chunkSize = swaple(chunkSize);
			riffSizeAccum += sizeof(chunkSize);

			if(riffSize - riffSizeAccum < chunkSize) { ENGINE_THROW("data chunk size does not fit into riff size"); }

			if(chunkHeader == "fmt ") {
				if(chunkSize != 16) { ENGINE_THROW("format chunk size must be 16 (PCM)"); }

				uint16_t formatTag;
				iss.read(reinterpret_cast<char *>(&formatTag), sizeof(formatTag));
				formatTag = swaple(formatTag);
				if(formatTag != 1) { ENGINE_THROW("format tag must be PCM"); }
				riffSizeAccum += sizeof(formatTag);

				uint16_t numChannels;
				iss.read(reinterpret_cast<char *>(&numChannels), sizeof(numChannels));
				numChannels = swaple(numChannels);
				if(numChannels != 1 && numChannels != 2) { ENGINE_THROW("number of channels must be 1 or 2"); }
				riffSizeAccum += sizeof(numChannels);
				asset.stereo = numChannels == 2;

				uint32_t sampleRate;
				iss.read(reinterpret_cast<char *>(&sampleRate), sizeof(sampleRate));
				sampleRate = swaple(sampleRate);
				riffSizeAccum += sizeof(sampleRate);
				asset.sampleRate = sampleRate;

				uint32_t avgByteRate;
				iss.read(reinterpret_cast<char *>(&avgByteRate), sizeof(avgByteRate));
				avgByteRate = swaple(avgByteRate);
				riffSizeAccum += sizeof(avgByteRate);

				uint16_t blockAlign;
				iss.read(reinterpret_cast<char *>(&blockAlign), sizeof(blockAlign));
				blockAlign = swaple(blockAlign);
				riffSizeAccum += sizeof(blockAlign);

				uint16_t bitsPerSample;
				iss.read(reinterpret_cast<char *>(&bitsPerSample), sizeof(bitsPerSample));
				bitsPerSample = swaple(bitsPerSample);
				if(bitsPerSample != 16) { ENGINE_THROW("bits per sample must be 16"); }
				riffSizeAccum += sizeof(bitsPerSample);

				uint8_t bytesPerSample = bitsPerSample >> 3;

				if(blockAlign != numChannels * bytesPerSample) { ENGINE_THROW("block align is incorrect"); }
				if(avgByteRate != sampleRate * blockAlign) { ENGINE_ERROR("average byte rate is incorrect"); }
			} else if(chunkHeader == "data") {
				std::string data(chunkSize, ' ');
				iss.read(&data[0], chunkSize);
				riffSizeAccum += chunkSize;

				asset.samples.resize(chunkSize / 2);
				for(uint32_t i = 0; i < chunkSize; i += 2) {
					auto sample = *reinterpret_cast<int16_t *>(&data[i]);
					sample = swaple(sample);
					asset.samples[i / 2] = sample;
				}

				if(chunkSize % 2 == 1) {
					if(riffSize - riffSizeAccum < 1) { ENGINE_THROW("data padding byte does not fit into riff size"); }
					iss.ignore(1);
					++riffSizeAccum;
				}
			} else if(chunkHeader == "fact") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "smpl") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "acid") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "id3 ") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "INFO") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "LIST") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else {
				std::stringstream ss;
				ss << "unrecognized chunk header `" << chunkHeader << "` at 0x" << std::hex << riffSizeAccum;
				ENGINE_THROW(ss.str());
			}
		}

		s << asset;
		return AssetName<AudioAsset>();
	};
	converters["gls"] = [] (const std::string &data, Serializer &s) {
		ShaderProgramAsset asset;
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

					if(directive == "shader") { /* shader stage */
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing shader type"); }

						if(args[0] == "vertex") {
							asset.shaders.emplace_back(ShaderType::Vertex, header.str());
						} else if(args[0] == "fragment") {
							asset.shaders.emplace_back(ShaderType::Fragment, header.str());
						} else { ENGINE_ERROR(iLine << ": unknown shader type `" << args[0] << '`'); }
					} else if(directive == "uniform") { /* uniform variable */
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing uniform type"); }
						if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing uniform name"); }
						asset.uniforms.emplace_back(args[1]);
						uniforms.emplace_back(args[0], args[1]);
					} else if(directive == "attrib") { /* vertex attribute*/
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing attribute type"); }
						if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing attribute name"); }
						attribs.emplace_back(args[0], args[1]);
					} else if(directive == "varying") { /* vertex->fragment interpolable */
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing varying interpolation method"); }
						if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing varying type"); }
						if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing varying name"); }
						varyings.emplace_back(args[0], args[1], args[2]);
					} else if(directive == "in") { /* input from previous pipeline stage */
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing program input key"); }
						if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing program input name"); }
						asset.ins.emplace_back(args[0], args[1]);
						uniforms.emplace_back("sampler2D", args[1]);
					} else if(directive == "gin") { /* global input */
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing program input key"); }
						if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing program input name"); }
						asset.globalIns.emplace_back(args[0], args[1]);
						uniforms.emplace_back("sampler2D", args[1]);
					} else if(directive == "out") { /* output to next pipeline stage */
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing program output type"); }
						if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing program output key"); }
						if(args[0] == "rgba8") {
							if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing program output name"); }
							asset.outs.emplace_back(ProgramOutputType::RGBA8, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "rgb16f") {
							if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing program output name"); }
							asset.outs.emplace_back(ProgramOutputType::RGB16F, args[1]);
							outs.emplace_back("vec3", args[2]);
						} else if(args[0] == "rgba16f") {
							if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing program output name"); }
							asset.outs.emplace_back(ProgramOutputType::RGBA16F, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "rgb32f") {
							if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing program output name"); }
							asset.outs.emplace_back(ProgramOutputType::RGB32F, args[1]);
							outs.emplace_back("vec3", args[2]);
						} else if(args[0] == "rgba32f") {
							if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing program output name"); }
							asset.outs.emplace_back(ProgramOutputType::RGBA32F, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "depth") {
							asset.outs.emplace_back(ProgramOutputType::Depth, args[1]);
						} else { ENGINE_ERROR(iLine << ": unknown program output type `" << args[0] << '`'); }
					} else if(directive == "gout") { /* global output */
						if(args.size() < 1) { ENGINE_ERROR(iLine << ": missing program global output type"); }
						if(args.size() < 2) { ENGINE_ERROR(iLine << ": missing program global output key"); }
						if(args[0] == "rgba8") {
							if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing program global output name"); }
							asset.globalOuts.emplace_back(ProgramOutputType::RGBA8, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "rgb16f") {
							if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing program global output name"); }
							asset.globalOuts.emplace_back(ProgramOutputType::RGB16F, args[1]);
							outs.emplace_back("vec3", args[2]);
						} else if(args[0] == "rgba16f") {
							if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing program global output name"); }
							asset.globalOuts.emplace_back(ProgramOutputType::RGBA16F, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "rgb32f") {
							if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing program global output name"); }
							asset.globalOuts.emplace_back(ProgramOutputType::RGB32F, args[1]);
							outs.emplace_back("vec3", args[2]);
						} else if(args[0] == "rgba32f") {
							if(args.size() < 3) { ENGINE_ERROR(iLine << ": missing program global output name"); }
							asset.globalOuts.emplace_back(ProgramOutputType::RGBA32F, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "depth") {
							asset.globalOuts.emplace_back(ProgramOutputType::Depth, args[1]);
						} else { ENGINE_ERROR(iLine << ": unknown program global output type `" << args[0] << '`'); }
					} else { ENGINE_ERROR(iLine << ": unknown directive `" << directive << '`'); }
				} else {
					if(asset.shaders.empty()) { header << line << '\n'; } else { asset.shaders.back().source += line + '\n'; }
				}
			}
		}

		for(auto &shader : asset.shaders) {
			std::string prepend;
			prepend += "#version 150\n";
			prepend += "#extension GL_ARB_explicit_attrib_location: enable\n";
			prepend += "precision highp float;\n";
			for(auto &uniform : uniforms) {
				prepend += "uniform " + std::get<0>(uniform) + ' ' + std::get<1>(uniform) + ";\n";
			}
			if(shader.type == ShaderType::Vertex) {
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
				int outLoc = 0;
				for(auto &out : outs) {
					prepend += "layout(location=" + std::to_string(outLoc++) + ") out " + std::get<0>(out) + ' ' + std::get<1>(out) + ";\n";
				}
			}
			shader.source = prepend + shader.source;
		}

		s << asset;
		return AssetName<ShaderProgramAsset>();
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
				std::stringstream ss;
				std::string type = converter->second(data, Serializer(ss));

				std::string name = file.substr(0, pos);
				FileSystem::CreateDir(buildPath + '/' + type);
				FileSystem::WriteFile(buildPath + '/' + type + '/' + name + ".asset", ss);
			} else { ENGINE_ERROR(file << ": no appropriate converter found"); }
		}
	}
	ENGINE_CONTINUE;
	return 0;
}
