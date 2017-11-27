#pragma once

#include <polar/asset/base.h>
#include <polar/asset/text.h>
#include <polar/support/shader/types.h>

namespace polar::asset {
	struct shaderinput : base {
		std::string key;
		std::string name;

		shaderinput() {}
		shaderinput(std::string key, std::string name) : key(key), name(name) {}
	};

	inline serializer &operator<<(serializer &s, const shaderinput &asset) {
		return s << asset.key << asset.name;
	}

	inline deserializer &operator>>(deserializer &s, shaderinput &asset) {
		return s >> asset.key >> asset.name;
	}

	struct shaderoutput : base {
		using outputtype = support::shader::outputtype;

		outputtype type;
		std::string key;

		shaderoutput() {}
		shaderoutput(outputtype type, std::string key) : type(type), key(key) {}
	};

	inline serializer &operator<<(serializer &s, const shaderoutput &asset) {
		return s << static_cast<uint8_t>(asset.type) << asset.key;
	}

	inline deserializer &operator>>(deserializer &s, shaderoutput &asset) {
		return s >> *reinterpret_cast<uint8_t *>(&asset.type) >> asset.key;
	}

	struct shader : base {
		using shadertype = support::shader::shadertype;

		shadertype type;
		std::string source;

		shader() {}
		shader(shadertype type, std::string source)
		    : type(type), source(source) {}
	};

	inline serializer &operator<<(serializer &s, const shader &asset) {
		return s << static_cast<uint8_t>(asset.type) << asset.source;
	}

	inline deserializer &operator>>(deserializer &s, shader &asset) {
		return s >> *reinterpret_cast<uint8_t *>(&asset.type) >> asset.source;
	}

	struct shaderprogram : base {
		std::vector<std::string> uniforms;
		std::vector<shaderinput> ins;
		std::vector<shaderoutput> outs;
		std::vector<shaderinput> globalIns;
		std::vector<shaderoutput> globalOuts;
		std::vector<shader> shaders;
	};

	inline serializer &operator<<(serializer &s, shaderprogram asset) {
		return s << asset.uniforms << asset.ins << asset.outs << asset.globalIns
		         << asset.globalOuts << asset.shaders;
	}

	inline deserializer &operator>>(deserializer &s, shaderprogram &asset) {
		return s >> asset.uniforms >> asset.ins >> asset.outs >>
		       asset.globalIns >> asset.globalOuts >> asset.shaders;
	}

	template<> inline std::string name<shaderprogram>() {
		return "shaderprogram";
	}
} // namespace polar::asset
