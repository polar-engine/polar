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

inline Serializer & operator<<(Serializer &s, const std::string &str) {
	s << static_cast<const std::uint32_t>(str.length());
	return s.write(str.data(), str.length());
}

template<typename T> inline Serializer & operator<<(Serializer &s, const std::vector<T> &vec) {
	s << static_cast<const std::uint32_t>(vec.size());
	for(auto &elem : vec) {
		s << elem;
	}
	return s;
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

inline Deserializer & operator>>(Deserializer &s, std::string &str) {
	std::uint32_t len;
	s >> len;
	str.clear();
	str.resize(len);
	return s.read(&str.at(0), len);
}

template<typename T> inline Deserializer & operator>>(Deserializer &s, std::vector<T> &vec) {
	std::uint32_t len;
	s >> len;
	vec.resize(len);
	for(auto &elem : vec) {
		s >> elem;
	}
	return s;
}
