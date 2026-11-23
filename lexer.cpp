#include "lexer.h"

#include <stdexcept>
#include <format>
//#include <print>

namespace asm_vm {
	kind Tokenizer::is_op(const std::string& word) {
		if(auto it = ops.find(word); it != ops.end()) {
			return it->second;
		} return kind::INVALID;
	}

	void Tokenizer::process_lit() {
		char c = source[cursor];
		if(c != '\'' && c != '"') throw std::runtime_error("invalid process lit call");
		char finish = c;
		std::string lit;

		lit += c;
		c = source[++cursor];

		while(
			cursor <= source.size()
		) {
			c = gc();
			if(c == finish)	break;

			lit += c;
			if(cursor == source.size() -1) throw std::runtime_error("lost str lit close");
			++cursor;
		}

		lit += finish;
		++cursor; // skipping clossing char

		tokens.emplace_back(Token{
			.kind = kind::LITERAL
		, .lit  = std::move(lit)
		});
	}

	void Tokenizer::process_word() {
		std::string word;

		word += gc();
		consume();

		while(cursor <= source.size() 
		&& (std::isalnum(gc()) || gc() == '_')
		) {
			word += gc();
			consume();
		}

		if(auto op = is_op(word); op != kind::INVALID) 
			tokens.emplace_back(Token{.kind = op, .lit = std::move(word)});
		else tokens.emplace_back(Token{.kind = kind::WORD, .lit = std::move(word)});
	}

	void Tokenizer::process_num() {
		std::string num;

		num += gc();
		consume();

		while(cursor <= source.size() 
		&& (std::isalnum(gc()) || gc() == '_')
		) {
			if(gc() == '_') continue; // ignoring _
			num += gc();
			consume();
		}

		tokens.emplace_back(Token{.kind = kind::LITERAL, .lit = std::move(num)});
	}

	std::vector<Token> Tokenizer::tokenize(std::string& source__) {
		source = source__;
		cursor = 0;
		while(cursor < source.size()) {

			char c = source[cursor];

			// word
			if(std::isalpha(c) || c == '_') {
				process_word();
				continue;
			}

			if(std::isdigit(c) || c == '-') {
				process_num();
				continue;
			}

			if(c == ',') {
				tokens.emplace_back(Token{.kind = kind::COMMA, .lit = ","});
				consume();
				continue;
			}
			if(c == ':') {
				tokens.emplace_back(Token{.kind = kind::COLON, .lit = ":"});
				consume();
				continue;
			}
			if(c == '?') {
				tokens.emplace_back(Token{.kind = kind::QMARK, .lit = "?"});
				consume();
				continue;
			}

			if(c == '\'' || c == '"') {
				process_lit();
				continue;
			}

			if(std::isspace(c)) {
				consume();
				continue;
			}

			throw std::runtime_error(std::format("Unexpected char: {}", c));
		} 

		tokens.shrink_to_fit();
		return std::move(tokens);
	} 

}
