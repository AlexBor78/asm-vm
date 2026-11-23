#include "vm.h"

#include "utils.h"
#include <print>
#include <format>
#include <bit>
#include <cstring>

namespace asm_vm {
	void ASM_VM::execute(const Program& prog) {
		init_vm(prog);
		while(cursor < prog.instructs.size()) {
			const auto& inst = prog.instructs[cursor];
			switch(inst.kind) {
				default: throw std::runtime_error(std::format(
					"Illegal instruction: {}", stringify(inst.kind)
				));

				case kind::PUSH_LIT: inst_push_lit(inst.arg); break;
				case kind::PUSH_MEM: inst_push_mem(inst.arg); break;
				case kind::POP_MEM:  inst_pop_mem(inst.arg); break;
				case kind::POP: 		 inst_pop(); break;

				case kind::ADD: 		 inst_add(); break;
				case kind::SUB: 		 inst_sub(); break;
				case kind::MUL: 		 inst_mul(); break;
				case kind::DIV: 		 inst_div(); break;

				case kind::NOT: 		 inst_not(); break;
				case kind::AND: 		 inst_and(); break;
				case kind::XOR: 		 inst_xor(); break;
				case kind::OR: 		 	 inst_or(); break;

				case kind::INC_STK:	 inst_inc_stk(); break;
				case kind::INC_MEM:	 inst_inc_mem(inst.arg); break;

				case kind::LEA: 		 inst_lea(inst.arg); break;
				case kind::CMP: 		 inst_cmp(); break;
				case kind::JMP: 		 inst_jmp(inst.arg); break;

				case kind::JE: 		 	 inst_je(inst.arg); break;
				case kind::JNE: 		 inst_jne(inst.arg); break;

				case kind::JL: 		 	 inst_jl(inst.arg); break;
				case kind::JLE: 		 inst_jle(inst.arg); break;
				case kind::JG: 		 	 inst_jg(inst.arg); break;
				case kind::JGE: 		 inst_jge(inst.arg); break;

				case kind::JB: 		 	 inst_jb(inst.arg); break;
				case kind::JBE: 		 inst_jbe(inst.arg); break;
				case kind::JA: 		 	 inst_ja(inst.arg); break;
				case kind::JAE: 		 inst_jae(inst.arg); break;

				case kind::PUTC: 		 			inst_putc(); break;
				case kind::PRINTS: 	 			inst_prints(); break;
				case kind::PRINT_STACK: 	inst_print_stack(); break;

				case kind::RET:			 return;
			}
		}
	}

	void ASM_VM::init_vm(const Program& prog) {
		if(prog.memory_size == 0) return;
		memory = std::make_unique<uint8_t[]>(prog.memory_size);
		for(const auto& var_pair : prog.vars) {
			const auto& var = var_pair.second;
			if(!var.init_value.has_value()) continue;
			if(var.init_value->is_plain) *(uint64_t*)(memory.get() + var.offset) = var.init_value->plain_data_64;
			else std::memcpy(
				memory.get() + var.offset
			, var.init_value->data.get()
			, var.init_value->data_sz
			);
		}
	}

	uint64_t ASM_VM::mem_get(uint64_t s) {
		return *(uint64_t*)(memory.get() + s);
	}

	void ASM_VM::mem_set(uint64_t dest, uint64_t val) {
		*(uint64_t*)(memory.get() + dest) = val;
	}

	// internal for recursive print
	static void print_stck(std::stack<uint64_t>& stck) {
		uint64_t v = stck.top();
		stck.pop();
		if(stck.size() > 0) print_stck(stck);
		std::println("{:016x}", v);
		stck.push(v);
	}

	void ASM_VM::stack_print() {
		std::println("Stack:");
		print_stck(stack);
	}

	void ASM_VM::stack_push(uint64_t val) {
		stack.push(val);
	}

	uint64_t ASM_VM::stack_pop() {
		uint64_t val = stack.top();
		stack.pop();
		return val;
	}

	void ASM_VM::cursor_set(uint64_t arg) {
		cursor = arg;
	}
	void ASM_VM::op() {
		++cursor;
	}

	void ASM_VM::inst_push_lit(uint64_t arg) {
		stack_push(arg); op();
	}

	void ASM_VM::inst_push_mem(uint64_t arg) {
		stack_push(mem_get(arg)); op();
	}


	inline static void throw_not_enough_ops_for(const std::string& name) {
		throw std::runtime_error(std::format(
			"not enough operands for '{}' in stack", name
		));
	}

	void ASM_VM::inst_pop_mem(uint64_t arg) {
		if(!expected_op()) throw_not_enough_ops_for("pop");
		mem_set(arg, stack_pop()); op();
	}

	void ASM_VM::inst_pop() {
		if(!expected_op()) throw_not_enough_ops_for("pop");
		stack_pop(); op();
	}

	void ASM_VM::inst_add() {
		if(!expected_two_ops()) throw_not_enough_ops_for("add");
		stack_push(stack_pop() + stack_pop()); op();
	}
	void ASM_VM::inst_sub() {
		if(!expected_two_ops()) throw_not_enough_ops_for("sub");
		stack_push(stack_pop() - stack_pop()); op();
	}
	void ASM_VM::inst_mul() {
		if(!expected_two_ops()) throw_not_enough_ops_for("mul");
		stack_push(stack_pop() * stack_pop()); op();
	}
	void ASM_VM::inst_div() {
		uint64_t v1 = stack_pop();
		uint64_t v2 = stack_pop();
		if(v2 == 0) throw std::runtime_error("Error: division by 0");
		stack_push(v1 / v2); op();
	}
	void ASM_VM::inst_not() {
		if(!expected_op()) throw_not_enough_ops_for("not");
		stack_push(~stack_pop()); op();
	}
	void ASM_VM::inst_and() {
		if(!expected_two_ops()) throw_not_enough_ops_for("and");
		stack_push(stack_pop() & stack_pop()); op();
	}
	void ASM_VM::inst_xor() {
		if(!expected_two_ops()) throw_not_enough_ops_for("xor");
		stack_push(stack_pop() ^ stack_pop()); op();
	}
	void ASM_VM::inst_or() {
		if(!expected_two_ops()) throw_not_enough_ops_for("or");
		stack_push(stack_pop() | stack_pop()); op();
	}

	void ASM_VM::inst_inc_stk() {
		if(!expected_op()) throw_not_enough_ops_for("inc");
		stack_push(stack_pop() + 1); op();
	}
	void ASM_VM::inst_inc_mem(uint64_t arg) {
		mem_set(arg, mem_get(arg) + 1); op();
	}
	// deprecated, will be removed in next commit
	void ASM_VM::inst_lea(uint64_t arg) {
		stack_push(mem_get(arg)); op();
	}
	void ASM_VM::inst_cmp() {
		if(!expected_two_ops()) throw_not_enough_ops_for("cmp");
		uint64_t v1 = stack_pop();
		uint64_t v2 = stack_pop();

		flags.eq = v1 == v2;
		flags.ab = v1 > v2;
		flags.gt = std::bit_cast<int64_t>(v1) > std::bit_cast<int64_t>(v2);

		stack_push(v2);
		stack_push(v1);
		op();
	}

	void ASM_VM::inst_jmp(uint64_t arg) {
		cursor_set(arg);
	}
	void ASM_VM::inst_je(uint64_t arg) {
		if(flags.eq) cursor_set(arg);
		else op();
	}
	void ASM_VM::inst_jne(uint64_t arg) {
		if(!flags.eq) cursor_set(arg);
		else op();
	}
	void ASM_VM::inst_jl(uint64_t arg) {
		if(!flags.gt) cursor_set(arg);
		else op();
	}
	void ASM_VM::inst_jle(uint64_t arg) {
		if(!flags.gt || flags.eq) cursor_set(arg);
		else op();
	}
	void ASM_VM::inst_jg(uint64_t arg) {
		if(flags.gt) cursor_set(arg);
		else op();
	}
	void ASM_VM::inst_jge(uint64_t arg) {
		if(flags.gt || flags.eq) cursor_set(arg);
		else op();
	}
	void ASM_VM::inst_jb(uint64_t arg) {
		if(!flags.ab) cursor_set(arg);
		else op();
	}
	void ASM_VM::inst_jbe(uint64_t arg) {
		if(!flags.ab || flags.eq) cursor_set(arg);
		else op();
	}
	void ASM_VM::inst_ja(uint64_t arg) {
		if(flags.ab) cursor_set(arg);
		else op();
	}
	void ASM_VM::inst_jae(uint64_t arg) {
		if(flags.ab || flags.eq) cursor_set(arg);
		else op();
	}

	void ASM_VM::inst_putc() {
		if(!expected_op()) throw_not_enough_ops_for("putc");
		std::print("{}", static_cast<char>(static_cast<uint8_t>(stack_pop())) );
		op();
	}
	void ASM_VM::inst_prints() {
		if(!expected_op()) throw_not_enough_ops_for("prints");
		throw std::runtime_error("prints: temporaly insupported");
		op();
	}
	void ASM_VM::inst_print_stack() {
		stack_print(); op();
	}
}
