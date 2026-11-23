#pragma once
#include <vector>
#include <string>
#include <string_view>

#include "common.h"

namespace asm_vm {
	class Tokenizer {
	private:
		size_t cursor{0};
		std::string source;
		std::vector<Token> tokens;
		kind is_op(const std::string&);
		void process_lit();
		void process_word();
		void process_num();
		void process_comment();

		inline char gc() const {
			return source[cursor];
		}

		inline char consume() {
			return source[++cursor];
		}

	public:
		std::vector<Token> tokenize(std::string&);
	};
}
