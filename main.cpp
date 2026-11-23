#include "lexer.h"
#include "parser.h"
#include "vm.h"
#include "utils.h"

#include <print>
#include <fstream>

int main() {
	std::ifstream file("code.asm");
	std::string code{
	 std::istreambuf_iterator<char>(file)
	, std::istreambuf_iterator<char>()
	};

	asm_vm::Tokenizer tok;
	std::println("Processing...");
	auto tokens = tok.tokenize(code);

	std::println("Printing tokens...");
	asm_vm::print(tokens);


	asm_vm::Parser parser;
	std::println("\nProcessing...");
	auto prog = parser.parse(tokens);

	std::println("Printing program:");
	asm_vm::print(prog);

	std::println("\nExecuting...");
	asm_vm::ASM_VM vm;
	vm.execute(prog);
}
