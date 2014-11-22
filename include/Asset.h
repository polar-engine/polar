#pragma once

#include <stdint.h>
#include "endian.h"

class Asset {
public:
	const std::string type = "invalid";
	Asset(const std::string &type) : type(type) {}
	virtual std::string Save() const = 0;
};
