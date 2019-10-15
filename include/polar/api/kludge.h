#pragma once

#include <variant>

namespace polar::api {
	class kludge {
	  public:
		enum class token_type {
			equals,
			accessor,
			number,
			identifier,
			invalid
		};

		class token {
		  public:
			using value_type = std::variant<std::monostate, Decimal, std::string>;
		  protected:
			token_type _type;
			value_type _value;

			token(token_type type, value_type value = {}) : _type(type), _value(value) {}
		  public:
			token() : token(token_type::invalid) {}

			static token equals() {
				return token{token_type::equals};
			}

			static token accessor() {
				return token{token_type::accessor};
			}

			static token number(Decimal x) {
				return token{token_type::number, x};
			}

			static token identifier(std::string x) {
				return token{token_type::identifier, x};
			}

			auto type() const {
				return _type;
			}

			template<typename T> T get() const {
				return std::get<T>(_value);
			}

			friend std::ostream & operator<<(std::ostream &os, const token &t) {
				switch(t.type()) {
				case token_type::equals:
					return os << "token::equals";
				case token_type::accessor:
					return os << "token::accessor";
				case token_type::number:
					return os << "token::number{" << t.get<Decimal>() << '}';
				case token_type::identifier:
					return os << "token::identifier{\"" << t.get<std::string>() << "\"}";
				default:
					return os << "token::invalid";
				}
			}

			friend bool operator==(const token &lhs, const token &rhs) {
				return lhs.type() == rhs.type() && lhs._value == rhs._value;
			}

			friend bool operator!=(const token &lhs, const token &rhs) {
				return !(lhs == rhs);
			}
		};

		enum class expr_type {
			number,
			identifier
		};

		class expr {
		  public:
			using value_type = std::variant<std::monostate, Decimal, std::string>;
		  protected:
			expr_type _type;
			std::vector<expr> _operands;
			value_type _value;

			expr(expr_type type, std::vector<expr> operands, value_type value) : _type(type), _operands(operands), _value(value) {}
		  public:
			static expr number(Decimal x) {
				return expr{expr_type::number, {}, x};
			}

			static expr identifier(std::string x) {
				return expr{expr_type::identifier, {}, x};
			}

			auto type() const {
				return _type;
			}

			template<typename T> T get() const {
				return std::get<T>(_value);
			}
		};

		using token_range = std::pair<std::vector<token>::const_iterator, std::vector<token>::const_iterator>;
	  protected:
		core::polar *engine = nullptr;
	  public:
		kludge(core::polar *engine) : engine(engine) {}

		std::pair<std::optional<char>, std::string_view>
		lex_char(std::string_view str) const {
			if(str.empty()) {
				return {{}, str};
			} else {
				char c = str[0];
				str.remove_prefix(1);
				return {{c}, str};
			}
		}

		std::string_view lex_whitespace(std::string_view str) const {
			for(;;) {
				auto [c, s] = lex_char(str);
				if(c && *c == ' ') {
					str = s;
				} else {
					break;
				}
			}
			return str;
		}

		std::pair<std::optional<uint_fast8_t>, std::string_view>
		lex_digit(std::string_view str) const {
			auto [c, s] = lex_char(str);
			if(c && *c >= '0' && *c <= '9') {
				return {{*c - '0'}, s};
			} else {
				return {{}, str};
			}
		}

		std::pair<std::optional<char>, std::string_view>
		lex_identifier_char_first(std::string_view str) const {
			auto [c, s] = lex_char(str);
			if(c && *c >= 'a' && *c <= 'z') {
				return {c, s};
			} else {
				return {{}, str};
			}
		}

		std::pair<std::optional<char>, std::string_view>
		lex_identifier_char(std::string_view str) const {
			auto [c, s] = lex_char(str);
			if(c && (*c >= 'a' && *c <= 'z' || *c >= '0' && *c <= '9')) {
				return {c, s};
			} else {
				return {{}, str};
			}
		}

		std::pair<std::optional<token>, std::string_view>
		lex_number(std::string_view str) const {
			str = lex_whitespace(str);

			Decimal r = 0;

			auto [d, s] = lex_digit(str);
			if(d) {
				r = *d;
				str = s;
			} else {
				return {{}, str};
			}

			for(;;) {
				auto [d, s] = lex_digit(str);
				if(d) {
					r = r * 10 + *d;
					str = s;
				} else {
					break;
				}
			}

			auto t = token::number(r);
			return {{t}, str};
		}

		std::pair<std::optional<token>, std::string_view>
		lex_identifier(std::string_view str) const {
			str = lex_whitespace(str);

			std::string r;

			auto [c, s] = lex_identifier_char_first(str);
			if(c) {
				r.push_back(*c);
				str = s;
			} else {
				return {{}, str};
			}

			for(;;) {
				auto [c, s] = lex_identifier_char(str);
				if(c) {
					r.push_back(*c);
					str = s;
				} else {
					break;
				}
			}

			auto t = token::identifier(std::string{r});
			return {{t}, str};
		}

		std::pair<std::optional<token>, std::string_view>
		lex_one(std::string_view str) const {
			str = lex_whitespace(str);

			{
				auto [c, s] = lex_char(str);
				if(c) {
					switch(*c) {
					case '=':
						return {token::equals(), s};
					case '.':
						return {token::accessor(), s};
					}
				}
			}
			{
				auto [t, s] = lex_number(str);
				if(t) {
					return {t, s};
				}
			}
			return lex_identifier(str);
		}

		auto lex(std::string_view str) const {
			std::vector<token> tokens;

			while(!str.empty()) {
				auto [t, s] = lex_one(str);
				if(t) {
					tokens.push_back(*t);
					str = s;
				} else {
					break;
				}
			}

			return tokens;
		}

		std::pair<std::optional<expr>, token_range>
		parse_identifier(token_range range) const {
			auto &t = *range.first;
			if(t.type() == token_type::identifier) {
				++range.first;
				return {{expr::identifier(t.get<std::string>())}, range};
			} else {
				return {{}, range};
			}
		}

		std::pair<std::optional<expr>, token_range>
		parse_one(token_range range) const {
			return parse_identifier(range);
		}

		auto parse_range(token_range range) const {
			std::vector<expr> exprs;

			for(;;) {
				auto [e, i] = parse_one(range);
				if(e) {
					exprs.emplace_back(*e);
					range = i;
				} else {
					break;
				}
			}

			return exprs;
		}

		auto parse(const std::vector<token> &tokens) const {
			return parse_range({tokens.begin(), tokens.end()});
		}

		auto parse(std::string_view str) const {
			return parse(lex(str));
		}

		bool exec(std::vector<expr> exprs) {
			bool ret = true;

			for(auto &e : exprs) {
				switch(e.type()) {
				case expr_type::identifier:
					if(e.get<std::string>() == "quit") {
						engine->quit();
					} else {
						ret = false;
					}
					break;
				default:
					ret = false;
					break;
				}
			}

			return ret;
		}

		bool exec(std::string_view str) {
			return exec(parse(str));
		}
	};
} // namespace polar::api
