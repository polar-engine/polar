#pragma once

#include <unordered_map>
#include "Asset.h"
#include "debug.h"

enum class ProgramInOutType : uint8_t {
	Invalid = 0,
	Color = 1,
	Depth = 2
};

struct ProgramInput {
	ProgramInOutType type;
	std::string name;
	ProgramInput(ProgramInOutType type, const std::string &name) : type(type), name(name) {}
	ProgramInput(ProgramInOutType type, std::string &&name) : type(type), name(name) {}
};

enum class ShaderType : uint8_t {
	Invalid  = 0,
	Vertex   = 1,
	Fragment = 2
};

struct Shader {
	const ShaderType type;
	std::string source;
	Shader(const ShaderType type, const std::string &source) : type(type), source(source) {}
	Shader(const ShaderType type, std::string &&source) : type(type), source(source) {}
};

class ShaderAsset : public Asset {
public:
	std::vector<ProgramInput> ins;
	std::vector<ProgramInOutType> outs;
	std::vector<Shader> shaders;

	static std::string Type() { return "shader"; }

	ShaderAsset() : Asset("shader") {}
	ShaderAsset(const std::vector<Shader> &shaders) : Asset("shader"), shaders(shaders) {}

	static ShaderAsset Load(const std::string &data) {
		ShaderAsset asset;
		const std::string::size_type dataLength = data.length();
		const char *sz = data.c_str();
		std::string::size_type pos = 0;

		if(dataLength - pos < 1) { ENGINE_THROW("missing program inputs count"); }
		const uint8_t insCount = sz[pos++];

		for(uint8_t i = 0; i < insCount; ++i) {
			if(dataLength - pos < sizeof(ProgramInOutType)) { ENGINE_THROW("missing program input type"); }
			const ProgramInOutType type = static_cast<ProgramInOutType>(sz[pos]);
			pos += sizeof(ProgramInOutType::Invalid);

			if(dataLength - pos < 1) { ENGINE_THROW("missing program input name length"); }
			const uint8_t length = sz[pos++];

			if(dataLength - pos < length) { ENGINE_THROW("invalid program input name length"); }
			std::string name(sz + pos, length);
			pos += length;

			asset.ins.emplace_back(type, name);
		}

		if(dataLength - pos < 1) { ENGINE_THROW("missing program outputs count"); }
		const uint8_t outsCount = sz[pos++];

		for(uint8_t i = 0; i < outsCount; ++i) {
			if(dataLength - pos < sizeof(ProgramInOutType)) { ENGINE_THROW("missing program output type"); }
			const ProgramInOutType type = static_cast<ProgramInOutType>(sz[pos]);
			pos += sizeof(ProgramInOutType::Invalid);

			asset.outs.emplace_back(type);
		}

		if(dataLength - pos < 1) { ENGINE_THROW("missing shader count"); }
		const uint8_t shaderCount = sz[pos++];

		for(uint8_t i = 0; i < shaderCount; ++i) {
			if(dataLength - pos < sizeof(ShaderType::Invalid)) { ENGINE_THROW("missing shader type"); }
			const ShaderType type = static_cast<ShaderType>(sz[pos]);
			pos += sizeof(ShaderType::Invalid);

			if(dataLength - pos < 2) { ENGINE_THROW("missing shader source length"); }
			const uint16_t length = swapbe(*reinterpret_cast<const uint16_t *>(sz + pos));
			pos += 2;

			if(dataLength - pos < length) { ENGINE_THROW("invalid shader source length"); }
			std::string source(sz + pos, length);
			pos += length;

			asset.shaders.emplace_back(type, source);
		}
		return asset;
	}
	std::string Save() const override final {
		std::ostringstream oss;

		oss << static_cast<uint8_t>(ins.size());
		for(auto &in : ins) {
			oss << static_cast<uint8_t>(in.type);
			oss << static_cast<uint8_t>(in.name.length());
			oss << in.name;
		}

		oss << static_cast<uint8_t>(outs.size());
		for(auto out : outs) {
			oss << static_cast<uint8_t>(out);
		}

		oss << static_cast<uint8_t>(shaders.size());
		for(auto &shader : shaders) {
			oss << static_cast<uint8_t>(shader.type);
			const uint16_t length = static_cast<uint16_t>(shader.source.length());
			const uint16_t beLength = swapbe(length);
			oss << std::string(reinterpret_cast<const char *>(&beLength), 2);
			oss << shader.source;
		}
		return oss.str();
	}
};
