#pragma once

#include <stdexcept>
#include "common.h"

namespace asm_vm {
	void print(const std::vector<Token>&);
	void print(const Token&);

	std::string stringify(const LitValue&);
	void print(const Program&);


	constexpr const char* stringify(kind kind) {
		switch(kind) {
			default: throw std::runtime_error("stringify: UNKNOWN token type");

			case kind::INVALID: return "INVALID";
			case kind::WORD: return "WORD";
			case kind::LITERAL: return "LITERAL";

			case kind::TYPE_DB: return "DB";
			case kind::TYPE_DW: return "DW";
			case kind::TYPE_DD: return "DD";
			case kind::TYPE_DQ: return "DQ";

			case kind::COMMA: return "COMMA";
			case kind::COLON: return "COLON";
			case kind::QMARK: return "QMARK";

			case kind::PUSH: return "PUSH";
			case kind::PUSH_LIT: return "PUSH_LIT";
			case kind::PUSH_MEM: return "PUSH_MEM";
			case kind::POP: return "POP";
			case kind::POP_MEM: return "POP_MEM";

			case kind::ADD: return "ADD";
			case kind::SUB: return "SUB";
			case kind::MUL: return "MUL";
			case kind::DIV: return "DIV";

			case kind::NOT: return "NOT";
			case kind::AND: return "AND";
			case kind::XOR: return "XOR";
			case kind::OR: return "OR";

			case kind::INC: return "INC";
			case kind::INC_STK: return "INC_STL";
			case kind::INC_MEM: return "INC_MEM";

			case kind::LEA: return "LEA";

			case kind::CMP: return "CMP";
			case kind::JMP: return "JMP";

			case kind::JE: 	return "JE";
			case kind::JNE: return "JNE";

			case kind::JL: 	return "JL";
			case kind::JLE: return "JLE";
			case kind::JG: 	return "JG";
			case kind::JGE: return "JGE";

			case kind::JB: 	return "JB";
			case kind::JBE: return "JBE";
			case kind::JA: 	return "JA";
			case kind::JAE: return "JAE";
			
			case kind::PUTC: 				return "PUTC";
			case kind::PRINTS:			return "PRINTS";
			case kind::PRINT_STACK:	return "PRINT_STACK";

			case kind::RET: return "RET";
		} throw std::runtime_error("stringify: UNKNOWN token type");
	}

	constexpr bool is_type(kind k) {
		return static_cast<int8_t>(k) >= static_cast<int8_t>(kind::TYPE_DB)
		&& static_cast<int8_t>(k) <= static_cast<int8_t>(kind::TYPE_DQ); 
	}

	constexpr bool is_op(kind k) {
		return static_cast<int8_t>(k) >= static_cast<int8_t>(kind::PUSH)
		&& static_cast<int8_t>(k) <= static_cast<int8_t>(kind::RET); 
	}
}
