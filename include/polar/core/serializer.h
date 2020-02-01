#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <polar/math/types.h>
#include <polar/util/endian.h>
#include <polar/util/raw_vector.h>
#include <set>
#include <string>

#define swapendian swapbe

namespace polar::core {
	class serializer {
	  private:
		std::ostream &stream;

	  public:
		serializer(std::ostream &stream) : stream(stream) {}

		template<typename T>
		inline serializer &write(const T *buf, const std::streamsize count) {
			stream.write((const char *)buf, count * sizeof(T));
			return *this;
		}
	};

	inline serializer &operator<<(serializer &s, const std::uint8_t i) {
		return s.write(reinterpret_cast<const char *>(&i), 1);
	}

	inline serializer &operator<<(serializer &s, const bool b) {
		return s.write(reinterpret_cast<const char *>(&b), 1);
	}

	inline serializer &operator<<(serializer &s, const std::uint16_t i) {
		std::uint16_t be = swapendian(i);
		return s.write(reinterpret_cast<const char *>(&be),
		               sizeof(std::uint16_t));
	}

	inline serializer &operator<<(serializer &s, const std::int16_t i) {
		std::int16_t be = swapendian(i);
		return s.write(reinterpret_cast<const char *>(&be),
		               sizeof(std::int16_t));
	}

	inline serializer &operator<<(serializer &s, const std::uint32_t i) {
		std::uint32_t be = swapendian(i);
		return s.write(reinterpret_cast<const char *>(&be),
		               sizeof(std::uint32_t));
	}

	inline serializer &operator<<(serializer &s, const std::uint64_t i) {
		std::uint64_t be = swapendian(i);
		return s.write(reinterpret_cast<const char *>(&be),
		               sizeof(std::uint64_t));
	}

	inline serializer &operator<<(serializer &s, const std::float_t f) {
		return s << *reinterpret_cast<const std::uint32_t *>(&f);
	}

	inline serializer &operator<<(serializer &s, const std::double_t f) {
		return s << *reinterpret_cast<const std::uint64_t *>(&f);
	}

	inline serializer &operator<<(serializer &s, const std::string &str) {
		s << static_cast<const std::uint32_t>(str.length());
		return s.write(str.data(), str.length());
	}

	template<typename T>
	inline serializer &operator<<(serializer &s, const std::optional<T> &opt) {
		if(opt) {
			return s << true << *opt;
		} else {
			return s << false;
		}
	}

	template<typename T, std::size_t N>
	inline serializer &operator<<(serializer &s, const std::array<T, N> &arr) {
		s << static_cast<const std::uint32_t>(arr.size());
		for(auto &elem : arr) { s << elem; }
		return s;
	}

	template<typename T>
	inline serializer &operator<<(serializer &s, const raw_vector<T> &vec) {
		s << static_cast<const std::uint32_t>(vec.size());
		s.write(vec.data(), vec.size());
		return s;
	}

	template<typename T>
	inline serializer &operator<<(serializer &s, const std::vector<T> &vec) {
		s << static_cast<const std::uint32_t>(vec.size());
		for(auto &elem : vec) { s << elem; }
		return s;
	}

	template<typename T>
	inline serializer &operator<<(serializer &s, const std::set<T> &set) {
		s << static_cast<const std::uint32_t>(set.size());
		for(auto &elem : set) { s << elem; }
		return s;
	}

	inline serializer &operator<<(serializer &s, const math::point2 &p) {
		return s << p.x << p.y;
	}

	inline serializer &operator<<(serializer &s, const math::point3 &p) {
		return s << p.x << p.y << p.z;
	}

	class deserializer {
	  private:
		std::istream &stream;

	  public:
		deserializer(std::istream &stream) : stream(stream) {}

		template<typename T>
		inline deserializer &read(T *buf, const std::streamsize count) {
			stream.read((char *)buf, count * sizeof(T));
			return *this;
		}
	};

	inline deserializer &operator>>(deserializer &s, std::uint8_t &i) {
		return s.read(reinterpret_cast<char *>(&i), 1);
	}

	inline deserializer &operator>>(deserializer &s, bool &b) {
		return s.read(reinterpret_cast<char *>(&b), 1);
	}

	inline deserializer &operator>>(deserializer &s, std::uint16_t &i) {
		std::uint16_t be;
		s.read(reinterpret_cast<char *>(&be), sizeof(std::uint16_t));
		i = swapendian(be);
		return s;
	}

	inline deserializer &operator>>(deserializer &s, std::int16_t &i) {
		std::int16_t be;
		s.read(reinterpret_cast<char *>(&be), sizeof(std::int16_t));
		i = swapendian(be);
		return s;
	}

	inline deserializer &operator>>(deserializer &s, std::uint32_t &i) {
		std::uint32_t be;
		s.read(reinterpret_cast<char *>(&be), sizeof(std::uint32_t));
		i = swapendian(be);
		return s;
	}

	inline deserializer &operator>>(deserializer &s, std::uint64_t &i) {
		std::uint64_t be;
		s.read(reinterpret_cast<char *>(&be), sizeof(std::uint64_t));
		i = swapendian(be);
		return s;
	}

	inline deserializer &operator>>(deserializer &s, std::float_t &f) {
		return s >> *reinterpret_cast<std::uint32_t *>(&f);
	}

	inline deserializer &operator>>(deserializer &s, std::double_t &f) {
		return s >> *reinterpret_cast<std::uint64_t *>(&f);
	}

	inline deserializer &operator>>(deserializer &s, std::string &str) {
		std::uint32_t len;
		s >> len;
		str.clear();
		str.resize(len);
		return s.read(str.data(), len);
	}

	template<typename T>
	inline deserializer &operator>>(deserializer &s, std::optional<T> &opt) {
		bool has_value;
		s >> has_value;

		if(has_value) {
			T x;
			s >> x;
			opt.emplace(x);
		} else {
			opt.reset();
		}

		return s;
	}

	template<typename T, std::size_t N>
	inline deserializer &operator>>(deserializer &s, std::array<T, N> &arr) {
		std::uint32_t len;
		s >> len;
		for(auto &elem : arr) { s >> elem; }
		return s;
	}

	template<typename T>
	inline deserializer &operator>>(deserializer &s, raw_vector<T> &vec) {
		std::uint32_t len;
		s >> len;
		vec.resize(len);
		s.read(vec.data(), len);
		return s;
	}

	template<typename T>
	inline deserializer &operator>>(deserializer &s, std::vector<T> &vec) {
		std::uint32_t len;
		s >> len;
		vec.resize(len);
		for(auto &elem : vec) { s >> elem; }
		return s;
	}

	template<typename T>
	inline deserializer &operator>>(deserializer &s, std::set<T> &set) {
		std::uint32_t len;
		s >> len;
		set.clear();
		for(std::size_t i = 0; i < len; ++i) {
			T elem;
			s >> elem;
			set.emplace(elem);
		}
		return s;
	}

	inline deserializer &operator>>(deserializer &s, math::point2 &p) {
		return s >> p.x >> p.y;
	}

	inline deserializer &operator>>(deserializer &s, math::point3 &p) {
		return s >> p.x >> p.y >> p.z;
	}
} // namespace polar::core
