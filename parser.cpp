#include "parser.h"
#include "utils.h"

#include <bit>
#include <format>
#include <limits>
#include <cstring>

inline std::unordered_map<std::string, int64_t> var_type_sz {
	{"db", 1}
,	{"dw", 2}
,	{"dd", 4}
,	{"dq", 8}
};

constexpr inline static int64_t get_var_type_sz(const std::string& str) {
	if(var_type_sz.contains(str)) return var_type_sz.at(str);
	return -1;
}

namespace asm_vm {
	LitValue Parser::make_litval(const std::string& str) {
		auto ptr = std::make_unique<uint8_t[]>(str.size());
		std::memcpy(ptr.get(), str.c_str(), str.size());
		return LitValue{
			.is_plain = false
		, .data = std::move(ptr)
		, .data_sz = str.size()
		};
	}

	LitValue Parser::make_litval(uint64_t val, uint sz) {
		return LitValue{
			.plain_data_sz = sz
		, .is_plain = true
		,	.plain_data = val // little-endian ONLY
		};
	}

	LitValue Parser::litval_addition(LitValue v1, LitValue v2) {
		size_t new_size{0};

		// new size = value 1 size
		if(v1.is_ptr()) new_size += v1.data_sz;
		else new_size += v1.plain_data_sz;

		// value 1 size, for further reuse
		int s1 = new_size;

		// new size += value 1 size
		if(v2.is_ptr()) new_size += v2.data_sz;
		else new_size += v2.plain_data_sz;

		// allocating :3
		auto ptr = std::make_unique<uint8_t[]>(new_size);
		
		// copying value 1
		if(v1.is_ptr()) std::memcpy(ptr.get(), v1.data.get(), v1.data_sz);
		else std::memcpy(ptr.get(), &v1.plain_data, s1);

		// copying value 2
		if(v2.is_ptr()) std::memcpy(
			ptr.get() + s1 // moving pointer to end of value 1
		, v2.data.get()
		, v2.data_sz);
		else std::memcpy(
			ptr.get() + s1 // moving pointer to end of value 1
		, &v2.plain_data
		, v2.plain_data_sz
		);

		return LitValue{
			.data = std::move(ptr)
		, .data_sz = new_size
		};
	}

	inline static void throw_invalid_str_lit(const std::string& str) {
		throw std::runtime_error(std::format(
			"Empty string IS Invalid string literal: {}", str
		));
	}
	inline static void throw_unknown_backslash_sequence(char c) {
		throw std::runtime_error(std::format(
			"Unknown backslash('\\') sequence: '\\{}'", c
		));
	}

	LitValue Parser::parse_str_lit(const std::string& str) {
		if(str == "''" || str == "\"\"") throw_invalid_str_lit(str);
		std::string res;
		for(size_t i = 1; i < str.size(); ++i) { // danger to use unsigned i
			char c = str[i];
			if(c == '\\') {
				c = str[++i]; // consuming backslash
				switch (c) {
					default: throw_unknown_backslash_sequence(c);
					case('\\'): res += '\\'; break;
					case('\''): res += '\''; break;
					case('n'): res += '\n'; break;
					case('t'): res += '\t'; break;
					case('b'): res += '\b'; break;
					case('"'): res += '"'; break;
				} ++i; // consuming the backslashed char
			} else res += c;
		} return make_litval(res);
	}

	inline static void throw_unknown_num_size(uint64_t num) {
		throw std::runtime_error(std::format(
		 "Unknown num size: {}", num
		));
	}
	inline static bool wrong_num_sz(uint64_t num, uint sz) {
		switch(sz) {
			default: throw_unknown_num_size(num);

			case 1: return num < std::numeric_limits<uint8_t>::min() 
									|| num > std::numeric_limits<uint8_t>::max();

			case 2: return num < std::numeric_limits<uint16_t>::min() 
									|| num > std::numeric_limits<uint16_t>::max();

			case 4: return num < std::numeric_limits<uint32_t>::min() 
									|| num > std::numeric_limits<uint32_t>::max();

			case 8: return num < std::numeric_limits<uint64_t>::min() 
									|| num > std::numeric_limits<uint64_t>::max();
		}
	}

	inline static void throw_too_big_init_val(uint sz, const std::string& str) {
		throw std::runtime_error(std::format(
			"Too big initial value for {} bytes var: {}", sz, str
		));
	}
	LitValue Parser::parse_num_lit(const std::string& str, uint sz) {
		int base = 10;
		switch (str.back()) {
			case 'b':	base = 2; break; // bin
			case 'h':	base = 16; break; // hex
			case 'q': case 'o': base = 8; break; // oct
		}

		if(str[0] == '-') {
			int64_t value = std::stoll(
				str.substr(0, base == 10 ? str.size() : str.size() -1)
			, nullptr // wharever is that
			, base
			); if(wrong_num_sz(value, sz)) throw_too_big_init_val(sz, str);
			return make_litval(std::bit_cast<uint64_t>(value), sz);
		} else {
			uint64_t value = std::stoull(
			 // without base postfix
				str.substr(0, base == 10 ? str.size() : str.size() -1)
			, nullptr // wharever is that
			, base
			); if(wrong_num_sz(value, sz)) throw_too_big_init_val(sz, str);
			return make_litval(value, sz);
		}
	}

	LitValue Parser::parse_lit_intern(const std::string& str, uint sz) {
		// string processing
		if(str[0] == '\'' || str[0] == '"') {
			return parse_str_lit(str);
		}

		// number processing
		return parse_num_lit(str, sz == 0 ? 8 : sz);
	}

	LitValue Parser::parse_lit(uint sz) {
		LitValue val = parse_lit_intern(consume().lit, sz);
		while(cursor < tokens.size()
		&& gc().kind == kind::COMMA) { consume(); // consume the comma
			val = litval_addition(std::move(val), parse_lit_intern(consume().lit));
		} return val;
	}

	void Parser::add_var(
		const std::string& name
	, int size
	, std::optional<LitValue> init_val
	) {

		// calculating the real (full) var size = var size + initial lit size
		int sz{size};
		if(init_val.has_value() && init_val->is_ptr()) sz += init_val->data_sz;

		// storing the var
		program.vars[name] = VarDecl{
			.name = name
		,	.offset = program.memory_size
		,	.size = size
		, .init_value = std::move(init_val)
		}; 

		// memory align by 8
		sz = program.memory_size + sz;
		if(sz != 0 && sz % 8 != 0) sz = (sz / 8 + 1) * 8;
		if(trace) debug(std::format("meory before: {}", sz));
		if(sz != 0 && sz % 8 != 0) sz = (sz / 8 + 1) * 8;
		if(trace) debug(std::format("sz after: {}", sz));

		program.memory_size = sz;
	}

	inline static void throw_var_redefinition(const std::string& name) {
		throw std::runtime_error(std::format(
			"Variable '{}' redifinition", name
		));
	}
	inline static void throw_label_redefinition(const std::string& name) {
		throw std::runtime_error(std::format(
			"Label '{}' redifinition", name
		));
	}
	inline static void throw_literal_too_large(const std::string& name) {
		throw std::runtime_error(std::format(
			"literal too large for var '{}'", name
		));
	}
	void Parser::parse_name() {
		std::string name = consume().lit; // consuming the name
		consume(); // skipping the colon

		// var
		if(is_type(gc().kind)) {
			auto sz = get_var_type_sz(gc().lit);
			consume(); // skipping var type
			if(pass == PassN::FIRST && contain_var(name)) throw_var_redefinition(name);
			if(gc_cmp(kind::QMARK)) { 
				consume(); // skipping the ?
				if(pass == PassN::FIRST) add_var(name, sz);
			} else {
				auto lit = parse_lit(sz);
				if(!lit.is_ptr() && sz < lit.plain_data_sz) throw_literal_too_large(name);
				if(pass == PassN::FIRST) add_var(name, sz, std::move(lit));
			}
		}

		// if just a label
		else if(pass == PassN::SECOND) {
			if(contain_label(name)) throw_label_redefinition(name);
			add_label(name);
		}
	}

	inline static bool no_arg_needed(kind k) {
		switch (k) {
			default: return false;
			case kind::ADD:
			case kind::SUB:
			case kind::MUL:
			case kind::DIV:
			case kind::MOD:
			case kind::NOT:
			case kind::AND:
			case kind::XOR:
			case kind::OR:
			case kind::CMP:
			case kind::PUTC:
			case kind::PRINTS:
			case kind::PRINT_STACK:
			case kind::RET:
				return true;
		}
	}

	inline static bool arg_needed(kind k) {
		switch (k) {
			default: return false;
			case kind::PUSH:
			case kind::JMP:
			case kind::JE:
			case kind::JNE:
			case kind::JL:
			case kind::JLE:
			case kind::JG:
			case kind::JGE:
			case kind::JB:
			case kind::JBE:
			case kind::JA:
			case kind::JAE:
				return true;
		}
	}

	inline static bool only_label_needed(kind k) {
		switch (k) {
			default: return false;
			// JMP
			case kind::JMP: // just jump
			case kind::JE:	// jump if equal
			case kind::JNE: // jump if not equal

			// signed
			case kind::JL:	// jump if less (<)
			case kind::JLE: // jump if less or equal (<=)
			case kind::JG:	// jump if grater (>)
			case kind::JGE: // jump if grater or equal (>=)
		
			// unsigned
			case kind::JB:	// jump if below (<)
			case kind::JBE: // jump if below or equal (<=)
			case kind::JA:	// jump if above (>)
			case kind::JAE: // jump if above or equal (>=)
				return true;
		}
	}

	inline static void throw_unknown_op(const Token& tok) {
		throw std::runtime_error(std::format(
			"Unknown op kind: {}, lit: '{}'", stringify(tok.kind), tok.lit
		));
	}
	void Parser::skip_op() {
		if(trace) debug("Skip OP");

		if(no_arg_needed(gc().kind)) {
			if(trace) debug(std::format("Just op: {}", stringify(gc().kind)));
			consume(); ++ops_count; return;
		}

		if(arg_needed(gc().kind)) {
			if(trace) debug(std::format("Op with arg {}: {}", stringify(gc().kind), gc().lit));
			consume();
			if(have_more()) consume();
			++ops_count; return;
		}
	
		if(gc_cmp(kind::INC) || gc_cmp(kind::POP)) { consume(); // skipping the op
			if(have_more() && expected_var()) consume(); // skipping the arg
			++ops_count; return;
		}

		throw_unknown_op(gc());
	}

	inline static void throw_expected_ptr_lit_push() {
		throw std::runtime_error("Expected not ptr literal for PUSH;\nif using '', use HEX");
	}
	inline static void throw_push_expected_var_lit() {
		throw std::runtime_error("After PUSH: Expected var or literal");
	}
	inline static void throw_expected_name(const std::string& op) {
		throw std::runtime_error(std::format(
			"Expected name for {}", op
		));
	}
	inline static void throw_undeclared_label(const std::string& label) {
		throw std::runtime_error(std::format(
			"Undeclared label: '{}'", label
		));
	}
	void Parser::parse_op() {
		if(gc_cmp(kind::PUSH)) { consume(); // skipping the op
			if(expected_var()) {
				append(kind::PUSH_MEM, get_var(consume().lit).offset);
				return;
			}	
			if(expected_lit()) {
				auto lit = parse_lit();
				if(lit.is_ptr()) throw_expected_ptr_lit_push();
				append(kind::PUSH_LIT, lit.plain_data_64);
				return;
			} throw_push_expected_var_lit();
		}

		if(gc_cmp(kind::POP)) { consume(); // skipping the op
			if(expected_var()) append(kind::POP_MEM, get_var(consume().lit).offset);
			else append(kind::POP);
			return;
		}

		if(gc_cmp(kind::INC)) { consume();
			if(have_more() && expected_var()) append(kind::INC_MEM, get_var(consume().lit).offset);
			else append(kind::INC_STK);
			return;
		}

		if(no_arg_needed(gc().kind)) {
			append(consume().kind);
			return;
		}

		if(only_label_needed(gc().kind)) {
			auto op = consume(); // consume the op
			if(!gc_cmp(kind::WORD)) throw_expected_name(gc().lit);
			if(!contain_label()) throw_undeclared_label(gc().lit);
			append(op.kind, get_label());
			consume(); return; // consuming the label & returning
		}

		throw_unknown_op(gc());
	}

	Program Parser::parse(std::vector<Token>& tokens__) {
		tokens = tokens__;

		// collecting var decls
		cursor = 0;
		pass = PassN::FIRST;
		while(have_more()) {
			if(is_name()) parse_name();
			else consume();
		}
		
		// collecting label decls
		cursor = 0;
		ops_count = 0;
		pass = PassN::SECOND;
		while(have_more()) {
			if(is_name()) parse_name();
			else skip_op(); // to set ops_count value right
		}

		// parsing assembler itself
		cursor = 0;
		pass = PassN::THIRD;
		while(have_more()) {
			if(is_op(gc().kind)) parse_op();
			else consume(); // skipping the name
		}

		return std::move(program);
	}
}
