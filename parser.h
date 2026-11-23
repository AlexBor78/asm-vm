#pragma once

#include "common.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <print>

inline static void debug(const std::string& str) {
	std::println("{}", str);
}

inline static bool is_that_name_lit(const std::string& str) {
	switch(str.back()) {
		case 'b': case 'h': case 'q': case 'o':
		return true;
	} return false;
}

namespace asm_vm {
	class Parser {
	private:
		std::vector<Token> tokens;
		bool trace{false};
		size_t cursor{0};
		int ops_count{0};
		Program program{};
		enum class PassN {
			FIRST
		, SECOND
		, THIRD
		} pass{PassN::FIRST}; // start from 1

	private: // internal logic
		LitValue make_litval(const std::string&);
		LitValue make_litval(uint64_t, uint sz);
		LitValue litval_addition(LitValue, LitValue);
		
		inline bool have_more() const		 	{ return cursor < tokens.size(); }
		inline auto& gc(this auto& self) 	{ return self.tokens.at(self.cursor);	}
		inline bool gc_cmp(kind k) const 	{ return gc().kind == k; }
		inline Token consume() 						{ return tokens[cursor++]; }
		inline auto& seek(this auto& self, int count = 1) { return self.tokens.at(self.cursor + count);	}

		inline bool contain_var(const std::string& name) const { return program.vars.contains(name); }
		inline bool contain_var() const { return contain_var(gc().lit); }

		inline bool contain_label(const std::string& name) const { return program.labels.contains(name); }
		inline bool contain_label() const {	return contain_label(gc().lit);	}

		inline bool expected_label() const { return gc_cmp(kind::WORD) && contain_label(); }
		inline bool expected_var() const { return gc_cmp(kind::WORD) && contain_var(); }
		inline bool expected_lit() const {
			if(gc_cmp(kind::LITERAL)) return true;
			if(!gc_cmp(kind::WORD)) return false;
			return is_that_name_lit(gc().lit);
		}

		inline auto& get_var(this auto& self, const std::string& name) { return self.program.vars.at(name); }
		inline auto& get_var(this auto& self) { return self.get_var(self.gc().lit);	}
		inline int64_t get_label(const std::string& name) const { return program.labels.at(name); }
		inline int64_t get_label() const {	return get_label(gc().lit); 	}

		LitValue parse_lit(uint sz = 0);
		LitValue parse_lit_intern(const std::string&, uint sz = 0);
		LitValue parse_str_lit(const std::string&);
		LitValue parse_num_lit(const std::string&, uint sz);

		inline void append(Instruction t) {	program.instructs.emplace_back(t); }
		inline void append(kind k, uint64_t a = 0) { append(Instruction{.kind = k,	.arg = a}); }

		inline void add_label(const std::string& name) {
			if(trace) debug(std::format("add_label: '{}' {}", name, ops_count ));
			program.labels[name] = ops_count;
		}

		void add_var(
			const std::string& name
		, int size
		, std::optional<LitValue> init_val = {}
		);

		void parse_op();
		void skip_op(); // for the first pass, to change ops_count;

		void parse_name();
		inline bool is_name() const {
			return gc_cmp(kind::WORD)
			&&		 have_more()
			&&		 seek().kind == kind::COLON;
		}

	public:
		Parser(bool __trace = false):
				trace(__trace)
		{}

		Program parse(std::vector<Token>&);
	};
}
