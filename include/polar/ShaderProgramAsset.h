#pragma once

#include <vector>
#include "debug.h"
#include "Asset.h"
#include "TextAsset.h"
#include "Serializer.h"

/******************** ShaderProgramInputAsset ********************
* String   key name
* String   buffer name
*/
struct ShaderProgramInputAsset : Asset {
	std::string key;
	std::string name;

	ShaderProgramInputAsset() {}
	ShaderProgramInputAsset(std::string key, std::string name) : key(key), name(name) {}
};

inline Serializer & operator<<(Serializer &s, const ShaderProgramInputAsset &asset) {
	return s << asset.key << asset.name;
}

inline Deserializer & operator>>(Deserializer &s, ShaderProgramInputAsset &asset) {
	return s >> asset.key >> asset.name;
}

enum class ProgramOutputType : uint8_t {
	Invalid = 0,
	Depth   = 1,
	RGB8    = 2,
	RGBA8   = 3,
	RGB16F  = 4,
	RGBA16F = 5,
	RGB32F  = 6,
	RGBA32F = 7
};

/******************** ShaderProgramOutputAsset ********************
 * uint8    type
 * String   key name
 */
struct ShaderProgramOutputAsset : Asset {
	ProgramOutputType type;
	std::string key;

	ShaderProgramOutputAsset() {}
	ShaderProgramOutputAsset(ProgramOutputType type, std::string key) : type(type), key(key) {}
};

inline Serializer & operator<<(Serializer &s, const ShaderProgramOutputAsset &asset) {
	return s << static_cast<uint8_t>(asset.type) << asset.key;
}

inline Deserializer & operator>>(Deserializer &s, ShaderProgramOutputAsset &asset) {
	return s >> *reinterpret_cast<uint8_t *>(&asset.type) >> asset.key;
}

enum class ShaderType : uint8_t {
	Invalid  = 0,
	Vertex   = 1,
	Fragment = 2
};

/******************** ShaderAsset ********************
 * uint8    type
 * String   source
 */
struct ShaderAsset : Asset {
	ShaderType type;
	std::string source;

	ShaderAsset() {}
	ShaderAsset(ShaderType type, std::string source) : type(type), source(source) {}
};

inline Serializer & operator<<(Serializer &s, const ShaderAsset &asset) {
	return s << static_cast<uint8_t>(asset.type) << asset.source;
}

inline Deserializer & operator>>(Deserializer &s, ShaderAsset &asset) {
	return s >> *reinterpret_cast<uint8_t *>(&asset.type) >> asset.source;
}

/******************** ShaderProgramAsset ********************
 * List<ShaderProgramInputAsset>    inputs to the program
 * List<ShaderProgramOutputAsset>   outputs of the program
 * List<ShaderProgramInputAsset>    global inputs to the program
 * List<ShaderProgramOutputAsset>   global outputs of the program
 * List<ShaderAsset>                shaders of the program
 */
struct ShaderProgramAsset : Asset {
	std::vector<std::string> uniforms;
	std::vector<ShaderProgramInputAsset> ins;
	std::vector<ShaderProgramOutputAsset> outs;
	std::vector<ShaderProgramInputAsset> globalIns;
	std::vector<ShaderProgramOutputAsset> globalOuts;
	std::vector<ShaderAsset> shaders;
};

inline Serializer & operator<<(Serializer &s, ShaderProgramAsset asset) {
	return s << asset.uniforms << asset.ins << asset.outs << asset.globalIns << asset.globalOuts << asset.shaders;
}

inline Deserializer & operator>>(Deserializer &s, ShaderProgramAsset &asset) {
	return s >> asset.uniforms >> asset.ins >> asset.outs >> asset.globalIns >> asset.globalOuts >> asset.shaders;
}

template<> inline std::string AssetName<ShaderProgramAsset>() { return "ShaderProgram"; }
