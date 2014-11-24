#pragma once

#include <unordered_map>
#include "Asset.h"
#include "debug.h"

enum class ShaderType : uint8_t {
	Invalid		= 0,
	Vertex		= 1,
	Fragment	= 2
};

class Shader {
public:
	const ShaderType type;
	std::string source;
	Shader(const ShaderType type, const std::string &source) : type(type), source(source) {}
	Shader(const ShaderType type, std::string &&source) : type(type), source(source) {}
};

class ShaderAsset : public Asset {
public:
	std::vector<Shader> shaders;
	static std::string Type() { return "shader"; }
	ShaderAsset(const std::vector<Shader> &shaders) : Asset("shader"), shaders(shaders) {}
	static ShaderAsset Load(const std::string &data) {
		std::vector<Shader> shaders;
		const std::string::size_type dataLength = data.length();
		std::string::size_type pos = 0;

		if(dataLength < 1) { ENGINE_THROW("missing shader count"); }
		const char *sz = data.c_str();
		const uint8_t count = sz[pos++];

		for(uint8_t i = 0; i < count; ++i) {
			if(dataLength - pos < sizeof(ShaderType::Invalid)) { ENGINE_THROW("missing shader type"); }
			const ShaderType type = static_cast<ShaderType>(sz[pos++]);

			if(dataLength - pos < 2) { ENGINE_THROW("missing shader source length"); }
			const uint16_t length = swapbe(*reinterpret_cast<const uint16_t *>(sz + pos));
			pos += 2;

			if(dataLength - pos < length) { ENGINE_THROW("invalid shader source length"); }
			std::string source(sz + pos, length);
			shaders.emplace_back(type, source);
		}
		return shaders;
	}
	std::string Save() const override final {
		ENGINE_THROW("not implemented");
		return "";
	}
};
