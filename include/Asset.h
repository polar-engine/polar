#pragma once

#include <cstdint>
#include <string>
#include "endian.h"

class Asset {
public:
	const std::string type = Type();
	static std::string Type() { return "text"; }
	Asset(const std::string &type) : type(type) {}
	virtual ~Asset() {}
	virtual std::string Save() const = 0;
};
