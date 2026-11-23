#pragma once

#include "common.h"
#pragma once

#include "common.h"

#include <stack>
#include <memory>

namespace asm_vm {
	class ASM_VM {
	private:
		size_t cursor{0};
		std::stack<uint64_t> stack;
		std::unique_ptr<uint8_t[]> memory;
		struct {
			bool eq{false};
			bool gt{false};
			bool ab{false};
		} flags{};


		inline bool expected_op() const {
			return !stack.empty();
		}
		inline bool expected_two_ops() const {
			return stack.size() >= 2;
		}

		void init_vm(const Program&);
		void cursor_set(uint64_t);
		void op();

		uint64_t mem_get(uint64_t);
		void mem_set(uint64_t, uint64_t);
		
		void stack_print();
		void stack_push(uint64_t);
		uint64_t stack_pop();

	private:
		void inst_push_lit(uint64_t);
		void inst_push_mem(uint64_t);
		void inst_pop_mem(uint64_t);
		void inst_pop();

		void inst_add();
		void inst_sub();
		void inst_mul();
		void inst_div();

		void inst_not();
		void inst_and();
		void inst_xor();
		void inst_or();

		void inst_inc_stk();
		void inst_inc_mem(uint64_t);

		void inst_cmp();
		void inst_jmp(uint64_t);
		void inst_je(uint64_t);
		void inst_jne(uint64_t);

		void inst_jl(uint64_t);
		void inst_jle(uint64_t);
		void inst_jg(uint64_t);
		void inst_jge(uint64_t);

		void inst_jb(uint64_t);
		void inst_jbe(uint64_t);
		void inst_ja(uint64_t);
		void inst_jae(uint64_t);

		void inst_putc();
		void inst_prints();
		void inst_print_stack();
	public:
		void execute(const Program&);
	};
}
