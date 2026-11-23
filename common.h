#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <map>

/**
 * A little stack VM, with simple asm
 */

namespace asm_vm {
	
	class Tokenizer;
	class Parser;

	enum class kind : int8_t {
	// internal
		INVALID
	, WORD
	, LITERAL

	// don't move the DB and DQ, add types between them (anotherways - will brok is_type)
	, TYPE_DB // 8 bit  (byte)
	, TYPE_DW // 16 bit (word)
	, TYPE_DD // 32 bit (double word)
	, TYPE_DQ // 64 bit (quad word)

	// for parsing
	, COMMA
	, COLON
	, QMARK
	
	// ops
	,	PUSH // ALWAYS FIRST OP
	,	PUSH_LIT
	,	PUSH_MEM

	, POP
	, POP_MEM

	// arithmetics 
	, ADD
	, SUB
	, MUL
	, DIV
	
	// logical ops
	, NOT
	, AND
	, XOR
	, OR


	, INC
	, INC_STK
	, INC_MEM

	, LEA

	// flow ops
	, CMP
	, JMP // jump

	, JE 	// jump if equal
	, JNE // jump if not equal

	// signed
	, JL 	// jump if less (<)
	, JLE // jump if less or equal (<=)
	, JG 	// jump if grater (>)
	, JGE // jump if grater or equal (>=)

	// unsigned
	, JB 	// jump if below (<)
	, JBE // jump if below or equal (<=)
	, JA 	// jump if above (>)
	, JAE // jump if above or equal (>=)


	, PUTC
	, PRINTS
	, PRINT_STACK


	, RET // ALWAYS LAST OP
	};

	inline std::unordered_map<std::string_view, kind> ops = {
	  {"db", kind::TYPE_DB}
	, {"dw", kind::TYPE_DW}
	, {"dd", kind::TYPE_DD}
	, {"dq", kind::TYPE_DQ}
						
  , {"push", kind::PUSH}
  , {"pop", kind::POP}

  , {"add", kind::ADD}
  , {"sub", kind::SUB}
  , {"mul", kind::MUL}
  , {"div", kind::DIV}

  , {"not", kind::NOT}
  , {"and", kind::AND}
  , {"xor", kind::XOR}
  , {"or", kind::OR}

  , {"inc", kind::INC}
  , {"lea", kind::LEA}

  , {"cmp", kind::CMP}

  , {"jmp", kind::JMP}
  , {"je", kind::JE}
  , {"jne", kind::JNE}

  , {"jl", kind::JL}
  , {"jle", kind::JLE}
  , {"jg", kind::JG}
  , {"jge", kind::JGE}

  , {"jb", kind::JB}
  , {"jbe", kind::JBE}
  , {"ja", kind::JA}
  , {"jae", kind::JAE}

  , {"putc", kind::PUTC}
  , {"prints", kind::PRINTS}
  , {"__print_stack", kind::PRINT_STACK}

  , {"ret", kind::RET}
	};

	struct Token {
		kind kind;
		std::string lit;
	};

	struct Instruction {
		kind kind;
		uint64_t arg;
	};

	struct LitValue {
	public: // data
		uint plain_data_sz{0};
		bool is_plain{false};
		std::unique_ptr<uint8_t[]> data{nullptr};
		union {
			size_t data_sz{0}; // equal to bytes count
			uint64_t plain_data;
			uint64_t plain_data_64;
			uint32_t plain_data_32;
			uint16_t plain_data_16;
			uint8_t plain_data_8;
		}; 

	public: // methods
		inline bool is_ptr() const noexcept {
			return !is_plain;
		}
	}; 

	struct VarDecl {
		std::string name;
		uint64_t offset{0}; // from memory start
		int size{-1}; 	// var size in bytes
		std::optional<LitValue> init_value{};
	};

	struct Program {
		std::vector<Instruction> instructs;
		std::map<std::string, VarDecl> vars;
		std::map<std::string, size_t> labels;

		size_t memory_size{0};
	};
}
