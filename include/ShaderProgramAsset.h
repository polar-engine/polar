#pragma once

#include <vector>
#include "debug.h"
#include "Asset.h"
#include "TextAsset.h"
#include "ListAsset.h"

/******************** ShaderProgramInputAsset ********************
* TextAsset   key name
* TextAsset   buffer name
*/
struct ShaderProgramInputAsset : Asset {
	TextAsset key;
	TextAsset name;

	ShaderProgramInputAsset() {}
	ShaderProgramInputAsset(TextAsset key, TextAsset name) : key(key), name(name) {}
};

inline std::ostream & operator<<(std::ostream &os, ShaderProgramInputAsset asset) {
	os << asset.key;
	os << asset.name;
	return os;
}

inline std::istream & operator>>(std::istream &is, ShaderProgramInputAsset &asset) {
	is >> asset.key;
	is >> asset.name;
	return is;
}

enum class ProgramOutputType : uint8_t {
	Invalid = 0,
	Color   = 1,
	Depth   = 2
};

/******************** ShaderProgramOutputAsset ********************
 * 1           type
 * TextAsset   key name
 */
struct ShaderProgramOutputAsset : Asset {
	ProgramOutputType type;
	TextAsset key;

	ShaderProgramOutputAsset() {}
	ShaderProgramOutputAsset(ProgramOutputType type, TextAsset key) : type(type), key(key) {}
};

inline std::ostream & operator<<(std::ostream &os, ShaderProgramOutputAsset asset) {
	os << static_cast<uint8_t>(asset.type);
	os << asset.key;
	return os;
}

inline std::istream & operator>>(std::istream &is, ShaderProgramOutputAsset &asset) {
	asset.type = static_cast<ProgramOutputType>(is.get());
	is >> asset.key;
	return is;
}

enum class ShaderType : uint8_t {
	Invalid  = 0,
	Vertex   = 1,
	Fragment = 2
};

/******************** ShaderAsset ********************
 * 1           type
 * TextAsset   source
 */
struct ShaderAsset : Asset {
	ShaderType type;
	TextAsset source;

	ShaderAsset() {}
	ShaderAsset(ShaderType type, TextAsset source) : type(type), source(source) {}
};

inline std::ostream & operator<<(std::ostream &os, ShaderAsset asset) {
	os << static_cast<uint8_t>(asset.type);
	os << asset.source;
	return os;
}

inline std::istream & operator>>(std::istream &is, ShaderAsset &asset) {
	asset.type = static_cast<ShaderType>(is.get());
	is >> asset.source;
	return is;
}

/******************** ShaderProgramAsset ********************
 * ListAsset<ShaderProgramInputAsset>    inputs to the program
 * ListAsset<ShaderProgramOutputAsset>   outputs of the program
 * ListAsset<ShaderProgramInputAsset>    global inputs to the program
 * ListAsset<ShaderProgramOutputAsset>   global outputs of the program
 * ListAsset<ShaderAsset>                shaders of the program
 */
struct ShaderProgramAsset : Asset {
	ListAsset<ShaderProgramInputAsset> ins;
	ListAsset<ShaderProgramOutputAsset> outs;
	ListAsset<ShaderProgramInputAsset> globalIns;
	ListAsset<ShaderProgramOutputAsset> globalOuts;
	ListAsset<ShaderAsset> shaders;

	ShaderProgramAsset() {}
};

inline std::ostream & operator<<(std::ostream &os, ShaderProgramAsset asset) {
	os << asset.ins;
	os << asset.outs;
	os << asset.globalIns;
	os << asset.globalOuts;
	os << asset.shaders;
	return os;
}

inline std::istream & operator>>(std::istream &is, ShaderProgramAsset &asset) {
	is >> asset.ins;
	is >> asset.outs;
	is >> asset.globalIns;
	is >> asset.globalOuts;
	is >> asset.shaders;
	return is;
}

template<> inline std::string AssetName<ShaderProgramAsset>() { return "ShaderProgram"; }
