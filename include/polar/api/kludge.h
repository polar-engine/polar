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
			using value_type = std::variant<std::monostate, math::decimal, std::string>;
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

			static token number(math::decimal x) {
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
					return os << "token::number { " << t.get<math::decimal>() << " }";
				case token_type::identifier:
					return os << "token::identifier { \"" << t.get<std::string>() << "\" }";
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
			identifier,
			assignment,
			access,
			system,
			system_getter,
			system_setter,
			component,
			component_getter,
			component_setter,
			builtin_engine,
			builtin_engine_quit,
			builtin_system,
			builtin_component,
			invalid
		};

		class expr {
		  public:
			using value_type = std::variant<
				std::monostate,
				math::decimal,
				std::string,
				std::type_index,
				system::base::accessor_type,
				component::base::accessor_type
			>;
		  protected:
			expr_type _type;
			std::vector<expr> _operands;
			value_type _value;

			expr(expr_type type, std::vector<expr> operands, value_type value) : _type(type), _operands(operands), _value(value) {}
			expr(expr_type type, std::vector<expr> operands) : expr(type, operands, {}) {}
			expr(expr_type type, value_type value) : expr(type, {}, value) {}
			expr(expr_type type) : expr(type, {}, {}) {}
		  public:
			static expr number(math::decimal x) {
				return expr{expr_type::number, x};
			}

			static expr identifier(std::string x) {
				return expr{expr_type::identifier, x};
			}

			static expr assignment(expr lhs, expr rhs) {
				return expr{expr_type::assignment, {lhs, rhs}};
			}

			static expr access(expr lhs, expr rhs) {
				return expr{expr_type::access, {lhs, rhs}};
			}

			static expr system(std::type_index ti) {
				return expr{expr_type::system, ti};
			}

			static expr system_getter(system::base::accessor_type accessor, expr cmp) {
				return expr{expr_type::system_getter, {cmp}, accessor};
			}

			static expr system_setter(system::base::accessor_type accessor, expr cmp, expr rhs) {
				return expr{expr_type::system_setter, {cmp, rhs}, accessor};
			}

			static expr component(std::type_index ti) {
				return expr{expr_type::component, ti};
			}

			static expr component_getter(component::base::accessor_type accessor, expr cmp) {
				return expr{expr_type::component_getter, {cmp}, accessor};
			}

			static expr component_setter(component::base::accessor_type accessor, expr cmp, expr rhs) {
				return expr{expr_type::component_setter, {cmp, rhs}, accessor};
			}

			static expr builtin_engine() {
				return expr{expr_type::builtin_engine};
			}

			static expr builtin_engine_quit() {
				return expr{expr_type::builtin_engine_quit};
			}

			static expr builtin_system() {
				return expr{expr_type::builtin_system};
			}

			static expr builtin_component() {
				return expr{expr_type::builtin_component};
			}

			auto type() const {
				return _type;
			}

			template<typename T> T get() const {
				return std::get<T>(_value);
			}

			expr & get(size_t i) {
				return _operands[i];
			}

			const expr & get(size_t i) const {
				return _operands[i];
			}

			std::ostream & write(std::ostream &os, size_t depth = 0) const {
				for(size_t i = 0; i < depth; ++i) { os << "  "; }

				switch(type()) {
				case expr_type::number:
					return os << "expr::number { " << get<math::decimal>() << " }";
				case expr_type::identifier:
					return os << "expr::identifier { \"" << get<std::string>() << "\" }";
				case expr_type::assignment:
					os << "expr::assignment {\n";
					get(0).write(os, depth + 1);
					os << '\n';
					get(1).write(os, depth + 1);
					os << '\n';
					for(size_t i = 0; i < depth; ++i) { os << "  "; }
					return os << '}';
				case expr_type::access:
					os << "expr::access {\n";
					get(0).write(os, depth + 1);
					os << '\n';
					get(1).write(os, depth + 1);
					os << '\n';
					for(size_t i = 0; i < depth; ++i) { os << "  "; }
					return os << '}';
				case expr_type::system:
					return os << "expr::system { " << get<std::type_index>().name() << " }";
				case expr_type::system_getter:
					os << "expr::system_getter {\n";
					for(size_t i = 0; i < depth + 1; ++i) { os << "  "; }
					os << "...\n";
					get(0).write(os, depth + 1);
					os << '\n';
					for(size_t i = 0; i < depth; ++i) { os << "  "; }
					return os << '}';
				case expr_type::system_setter:
					os << "expr::system_setter {\n";
					for(size_t i = 0; i < depth + 1; ++i) { os << "  "; }
					os << "...\n";
					get(0).write(os, depth + 1);
					os << '\n';
					get(1).write(os, depth + 1);
					os << '\n';
					for(size_t i = 0; i < depth; ++i) { os << "  "; }
					return os << '}';
				case expr_type::component:
					return os << "expr::component { " << get<std::type_index>().name() << " }";
				case expr_type::component_getter:
					os << "expr::component_getter {\n";
					for(size_t i = 0; i < depth + 1; ++i) { os << "  "; }
					os << "...\n";
					get(0).write(os, depth + 1);
					os << '\n';
					for(size_t i = 0; i < depth; ++i) { os << "  "; }
					return os << '}';
				case expr_type::component_setter:
					os << "expr::component_setter {\n";
					for(size_t i = 0; i < depth + 1; ++i) { os << "  "; }
					os << "...\n";
					get(0).write(os, depth + 1);
					os << '\n';
					get(1).write(os, depth + 1);
					os << '\n';
					for(size_t i = 0; i < depth; ++i) { os << "  "; }
					return os << '}';
				default:
					return os << "expr::invalid";
				}
			}

			friend std::ostream & operator<<(std::ostream &os, const expr &e) {
				return e.write(os);
			}
		};

		using token_range = std::pair<std::vector<token>::const_iterator, std::vector<token>::const_iterator>;
	  protected:
		core::polar &engine = nullptr;
	  public:
		kludge(core::polar &engine) : engine(engine) {}

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
			if(c && ((*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9'))) {
				return {c, s};
			} else {
				return {{}, str};
			}
		}

		std::pair<std::optional<token>, std::string_view>
		lex_number(std::string_view str) const {
			str = lex_whitespace(str);

			math::decimal r = 0;

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

		std::pair<std::optional<token>, token_range>
		parse_token(token_range range) const {
			if(range.first != range.second) {
				auto t = *range.first;
				++range.first;
				return {{t}, range};
			} else {
				return {{}, range};
			}
		}

		std::pair<std::optional<expr>, token_range>
		parse_number(token_range range) const {
			auto [x, r_x] = parse_token(range);
			if(x && x->type() == token_type::number) {
				return {{expr::number(x->get<math::decimal>())}, r_x};
			} else {
				return {{}, range};
			}
		}

		std::pair<std::optional<expr>, token_range>
		parse_identifier(token_range range) const {
			auto [id, r_id] = parse_token(range);
			if(id && id->type() == token_type::identifier) {
				return {{expr::identifier(id->get<std::string>())}, r_id};
			} else {
				return {{}, range};
			}
		}

		std::pair<std::optional<expr>, token_range>
		parse_assignment(token_range range) const {
			auto r = range;

			auto [lhs, r_lhs] = parse_access(r);
			if(lhs) {
				r = r_lhs;
			} else {
				return {{}, range};
			}

			auto [eq, r_eq] = parse_token(r);
			if(eq && eq->type() == token_type::equals) {
				r = r_eq;
			} else {
				return {{}, range};
			}

			auto [rhs, r_rhs] = parse_one(r);
			if(rhs) {
				r = r_rhs;
			} else {
				return {{}, range};
			}

			return {{expr::assignment(*lhs, *rhs)}, r};
		}

		std::pair<std::optional<expr>, token_range>
		parse_access_rhs(token_range range) const {
			auto r = range;

			auto [acc, r_acc] = parse_token(r);
			if(acc && acc->type() == token_type::accessor) {
				r = r_acc;
			} else {
				return {{}, range};
			}

			auto [rhs, r_rhs] = parse_identifier(r);
			if(rhs) {
				return {rhs, r_rhs};
			} else {
				return {{}, range};
			}
		}

		std::pair<std::optional<expr>, token_range>
		parse_access(token_range range) const {
			auto [lhs, r_lhs] = parse_identifier(range);
			if(lhs) {
				range = r_lhs;
			} else {
				return {{}, range};
			}

			for(;;) {
				auto [rhs, r_rhs] = parse_access_rhs(range);
				if(rhs) {
					lhs = expr::access(*lhs, *rhs);
					range = r_rhs;
				} else {
					break;
				}
			}

			return {lhs, range};
		}

		std::pair<std::optional<expr>, token_range>
		parse_one(token_range range) const {
			auto [assign, r_assign] = parse_assignment(range);
			if(assign) {
				return {assign, r_assign};
			}

			auto [x, r_x] = parse_number(range);
			if(x) {
				return {x, r_x};
			}

			return parse_access(range);
		}

		auto parse_range(token_range range) const {
			std::vector<expr> exprs;

			for(;;) {
				auto [e, r] = parse_one(range);
				if(e) {
					exprs.emplace_back(*e);
					range = r;
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

		bool reduce_access(expr &e) const {
			auto api_system = engine.get<system::api>().lock();

			bool ret = true;

			auto &lhs = e.get(0);
			auto &rhs = e.get(1);
			auto s = rhs.get<std::string>();

			ret &= reduce(lhs);

			switch(lhs.type()) {
			case expr_type::system:
				if(api_system) {
					if(auto accessor = api_system->get_system_accessor(lhs.get<std::type_index>(), s)) {
						e = expr::system_getter(*accessor, lhs);
					} else {
						ret = false;
					}
				}
				break;
			case expr_type::component:
				if(api_system) {
					if(auto accessor = api_system->get_component_accessor(lhs.get<std::type_index>(), s)) {
						e = expr::component_getter(*accessor, lhs);
					} else {
						ret = false;
					}
				}
				break;
			case expr_type::builtin_engine:
				if(s == "quit") {
					e = expr::builtin_engine_quit();
				} else {
					ret = false;
				}
				break;
			case expr_type::builtin_system:
				if(api_system) {
					if(auto ti = api_system->get_system_by_name(s)) {
						e = expr::system(*ti);
					} else {
						ret = false;
					}
				}
				break;
			case expr_type::builtin_component:
				if(api_system) {
					if(auto ti = api_system->get_component_by_name(s)) {
						e = expr::component(*ti);
					} else {
						ret = false;
					}
				}
				break;
			default:
				break;
			}

			return ret;
		}

		bool reduce(expr &e) const {
			bool ret = true;

			switch(e.type()) {
			case expr_type::identifier: {
				auto s = e.get<std::string>();
				if(s == "engine") {
					e = expr::builtin_engine();
				} else if(s == "system") {
					e = expr::builtin_system();
				} else if(s == "component") {
					e = expr::builtin_component();
				}
				break;
			}
			case expr_type::assignment: {
				auto &lhs = e.get(0);
				auto &rhs = e.get(1);
				reduce(lhs);
				reduce(rhs);
				switch(lhs.type()) {
				case expr_type::system_getter:
					e = expr::system_setter(lhs.get<system::base::accessor_type>(), lhs, rhs);
					break;
				case expr_type::component_getter:
					e = expr::component_setter(lhs.get<component::base::accessor_type>(), lhs, rhs);
					break;
				default:
					break;
				}
				break;
			}
			case expr_type::access:
				ret &= reduce_access(e);
				break;
			default:
				break;
			}

			return ret;
		}

		bool reduce(std::vector<expr> &exprs) {
			bool ret = true;

			for(auto &e : exprs) {
				ret &= reduce(e);
			}

			return ret;
		}

		enum class exec_error {
			type_mismatch
		};

		using exec_type = std::variant<std::monostate, exec_error, math::decimal>;

		exec_type exec_one(expr &e) {
			exec_type ret;

			if(!reduce(e)) {
				return ret;
			}

			switch(e.type()) {
			case expr_type::number:
				return e.get<math::decimal>();
			case expr_type::system_getter:
				if(auto getter = e.get<system::base::accessor_type>().getter) {
					auto ti = e.get(0).get<std::type_index>();
					if(auto sys = engine.get(ti).lock()) {
						ret = getter.value()(sys.get());
					}
				}
				break;
			case expr_type::system_setter: {
				auto rhs = exec_one(e.get(1));
				if(!std::holds_alternative<math::decimal>(rhs)) {
					return exec_error::type_mismatch;
				}

				if(auto setter = e.get<system::base::accessor_type>().setter) {
					auto ti = e.get(0).get(0).get<std::type_index>();
					auto rhs_value = std::get<math::decimal>(rhs);
					if(auto sys = engine.get(ti).lock()) {
						setter.value()(sys.get(), rhs_value);
					}
				}
				break;
			}
			case expr_type::component_getter:
				if(auto getter = e.get<component::base::accessor_type>().getter) {
					auto ti = e.get(0).get<std::type_index>();
					auto ti_range = engine.objects.get<core::index::ti>().equal_range(ti);
					for(auto ti_it = ti_range.first; ti_it != ti_range.second; ++ti_it) {
						ret = getter.value()(ti_it->ptr.get());
					}
				}
				break;
			case expr_type::component_setter: {
				auto rhs = exec_one(e.get(1));
				if(!std::holds_alternative<math::decimal>(rhs)) {
					return exec_error::type_mismatch;
				}

				if(auto setter = e.get<component::base::accessor_type>().setter) {
					auto ti = e.get(0).get(0).get<std::type_index>();
					auto rhs_value = std::get<math::decimal>(rhs);
					auto it_range = engine.objects.get<core::index::ti>().equal_range(ti);
					for(auto ti_it = it_range.first; ti_it != it_range.second; ++ti_it) {
						setter.value()(ti_it->ptr.get(), rhs_value);
					}
				}
				break;
			}
			case expr_type::builtin_engine_quit:
				engine.quit();
				break;
			default:
				break;
			}

			return ret;
		}

		exec_type exec(std::vector<expr> &exprs) {
			exec_type ret;
			for(auto &e : exprs) {
				ret = exec_one(e);
			}
			return ret;
		}

		exec_type exec(std::vector<expr> &&exprs) {
			return exec(exprs);
		}

		exec_type exec(std::string_view str) {
			return exec(parse(str));
		}
	};
} // namespace polar::api
