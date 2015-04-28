#pragma once

#include <cstdint>
#include <cmath>
#include <string>
#include <iostream>
#include "endian.h"

class Serializer {
private:
	std::ostream &stream;
public:
	Serializer(std::ostream &stream) : stream(stream) {}

	inline Serializer & write(const char *buf, const std::streamsize count) {
		stream.write(buf, count);
		return *this;
	}
};

inline Serializer & operator<<(Serializer &s, const std::uint8_t i) {
	return s.write(reinterpret_cast<const char *>(&i), 1);
}

inline Serializer & operator<<(Serializer &s, const bool b) {
	return s.write(reinterpret_cast<const char *>(&b), 1);
}

inline Serializer & operator<<(Serializer &s, const std::uint32_t i) {
	std::uint32_t be = swapbe(i);
	return s.write(reinterpret_cast<const char *>(&be), sizeof(std::uint32_t));
}

inline Serializer & operator<<(Serializer &s, const std::float_t f) {
	return s << *reinterpret_cast<const std::uint32_t *>(&f);
}

inline Serializer & operator<<(Serializer &s, const std::string str) {
	return s.write(str.data(), str.length());
}

class Deserializer {
private:
	std::istream &stream;
public:
	Deserializer(std::istream &stream) : stream(stream) {}

	inline Deserializer & read(char *buf, const std::streamsize count) {
		stream.read(buf, count);
		return *this;
	}
};

inline Deserializer & operator>>(Deserializer &s, std::uint8_t &i) {
	return s.read(reinterpret_cast<char *>(&i), 1);
}

inline Deserializer & operator>>(Deserializer &s, bool &b) {
	return s.read(reinterpret_cast<char *>(&b), 1);
}

inline Deserializer & operator>>(Deserializer &s, std::uint32_t &i) {
	std::uint32_t be;
	s.read(reinterpret_cast<char *>(&be), sizeof(std::uint32_t));
	i = swapbe(be);
	return s;
}

inline Deserializer & operator>>(Deserializer &s, std::float_t &f) {
	return s >> *reinterpret_cast<std::uint32_t *>(&f);
}
