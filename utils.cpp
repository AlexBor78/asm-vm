#include "utils.h"

#include <print>

#define DEBUG

inline static void debug(const std::string& str) {
#ifdef DEBUG
	std::println("{}", str);
#endif
}

namespace asm_vm {

	void print(const std::vector<Token>& tokens) {
		for(const auto& tok : tokens) {
			print(tok);
			std::println();
		} 
	}

	void print(const Token& tok) {
		std::print(
			"{{kind = {}, lit = '{}'}}"
		, stringify(tok.kind)
		, tok.lit
		);
	}

	std::string stringify(const LitValue& lit) {
		if(lit.is_ptr()) {
			std::string hex;
			for(size_t i = 0; i < lit.data_sz; ++i)	hex += std::format("{:02x}", lit.data[i]);
			return hex;
		} else if(lit.is_plain) switch (lit.plain_data_sz) {
			default: {
				debug("called the plain_data thing");
				throw std::runtime_error(std::format(
				"stringify LitValue: unknown plain_data_sz: {}", lit.plain_data_sz
			));}
			case (1): return std::format("{:02x}", lit.plain_data_8);
			case (2): return std::format("{:04x}", lit.plain_data_16);
			case (4): return std::format("{:08x}", lit.plain_data_32);
			case (8): return std::format("{:016x}", lit.plain_data_64);
		} else throw std::runtime_error("stringify lit_value: reached unreachable");
	}

	void print(const Program& prog) {
		// quick info
		std::println("Qick info:");
		std::println("vars count: {}", prog.vars.size());
		std::println("labels count: {}", prog.labels.size());
		std::println("instructs count: {}", prog.instructs.size());

		
		// printing vars tables
		std::println("\nVariables:");
		for(const auto& var_pair : prog.vars) {
			const auto& var = var_pair.second;
			std::println("'{:8}'\tz: {};\toffset: {};\ti_v: {}"
				, var.name
				, var.size
				, var.offset
				//, var.signed_ ? "true" : "false"
				, var.init_value.has_value() ? stringify(*var.init_value) : "none"
			);
		}
		
		// printing the labels table
		std::println("\nLabels:");
		for(const auto& label : prog.labels) {
			std::println("'{:8}':\t{}", label.first, label.second);
		}
		
		// printing the asm ops
		std::println("\nInstructoins:");
		for(const auto& inst : prog.instructs)
			std::println("{} {:016x}", stringify(inst.kind), inst.arg);
	}

}
