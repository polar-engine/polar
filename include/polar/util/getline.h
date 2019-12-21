#pragma once

#include <istream>
#include <sstream>
#include <string>
#include <unordered_set>

inline std::istream & getline(std::istream &is, std::string &line) {
	line.clear();
	std::istream::sentry sentry(is, true);
	std::streambuf *sb = is.rdbuf();

	while(true) {
		int c = sb->sbumpc();
		switch(c) {
		case '\n':
			return is;
		case '\r':
			if(sb->sgetc() == '\n') { sb->sbumpc(); }
			return is;
		case EOF:
			if(line.empty()) { is.setstate(std::ios::eofbit); }
			return is;
		default:
			line += static_cast<char>(c);
			break;
		}
	}
}

inline std::istream & get_component(std::istream &is, std::string &component, std::unordered_set<char> delims) {
	component.clear();

	if(is.eof()) {
		is.setstate(std::ios::failbit);
		return is;
	}

	std::istream::sentry sentry(is, true);
	std::streambuf *sb = is.rdbuf();

	while(true) {
		int c = sb->sbumpc();

		switch(c) {
		case EOF:
			is.setstate(std::ios::eofbit);
			return is;
		default:
			if(delims.find(c) != delims.end()) {
				return is;
			} else {
				component += static_cast<char>(c);
			}
			break;
		}
	}
}
