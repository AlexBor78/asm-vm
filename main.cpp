#include "lexer.h"
#include "parser.h"
#include "vm.h"
#include "utils.h"

#include <print>
#include <fstream>
#include <iostream>

bool verbose{false};

void print_help(const char* base) {
	std::cerr
	<< "Usage: " << base << " [OPTIONS]... SOURCE...\n"
	<< "Compile the sources and executes them (JIT)\n"
	<< std::endl // new line and flush
	<< "Flags list:\n"
	<< "-h,\t--help:\n"
	<< "\t\tdisplay this help and exit\n"
	<< "\t--verbose\n"
	<< "\t\texplain what is being done\n"
	<< std::flush;
}

int process_file(const std::string& file_path) { try {
	if(verbose) std::println("\nProcessing {}...", file_path);

	// readding whole file
	std::ifstream file(file_path);
	std::string code{
	 std::istreambuf_iterator<char>(file)
	, std::istreambuf_iterator<char>()
	};

	if(!file) {
		std::cerr << "no such file or directory: '"
		<< file_path << "'" << std::endl;
		return -1;
	}


	/**TODO
	 * Pass the verbose to lexer, parser so it will follow it on it's debug output
	 * INSTEAD of using -DDEBUG (or #define in code)
	 */

	asm_vm::Tokenizer tok;
	if(verbose) std::println("Processing...");
	auto tokens = tok.tokenize(code);

 	if(verbose)	std::println("Printing tokens...");
	if(verbose) asm_vm::print(tokens);


	asm_vm::Parser parser;
 	if(verbose)	std::println("\nProcessing...");
	auto prog = parser.parse(tokens);

 	if(verbose)	{
		std::println("Printing program:");
		asm_vm::print(prog);
	}

 	std::println("\nExecuting {}...", file_path);
	asm_vm::ASM_VM vm;
	vm.execute(prog);

	return 0;
	} catch (const std::exception& e) {
		std::cerr << "\n\nERROR in file '" << file_path << "':\n"
			<< e.what() << std::endl;
		return -1;
	} return 0;
}

int main(int argc, char** argv) { try {
	
	// flags
	bool help{false};

	// parsing the args
	std::vector<std::string> files;
	bool no_flags{false};
	for(int i = 1; i < argc; ++i) {

		// parsing flag
		if(!no_flags && *argv[i] == '-') {
			std::string arg{argv[i]};
			if(arg == "--") {
				no_flags = true;
				continue;
			}

			if(arg == "--verbose") {
				verbose = true;
				continue;
			}
			
			// fine for now
			if(arg == "-h" || arg == "--help") {
				help = true;
				continue;
			}

			std::cerr << argv[0] << ": invalid option -- '" << argv[i] << "'" << std::endl;;
			std::cerr << "Try '" << argv[0] << " --help' for more information." << std::endl;;
			return -1;
			
		} else files.emplace_back(argv[i]);
	}

	// if --help
	if(help
	|| (files.empty() && !verbose)) {
		print_help(argv[0]);
		return 0;
	}

	if(files.empty()) {
		std::cerr << argv[0] << ": No input files" << std::endl;;
		std::cerr << "Try '" << argv[0] << " --help' for more information." << std::endl;;
	}

	// processing each file; exiting on failure
	for(const auto& file : files)	if(process_file(file)) return -1;
	

	} catch(const std::exception& e) {
		// using std::pritln for that is ugly
		std::cerr << "\n\nERROR: "
			<< e.what() << std::endl;
	}
}
