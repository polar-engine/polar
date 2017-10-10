#pragma once

#include <istream>
#include <sstream>
#include <string>

std::istream &getline(std::istream &is, std::string &line) {
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
