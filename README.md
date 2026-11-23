# Quick Start
```bash
git clone https://github.com/AlexBor78/asm-vm && cd asm-vm
make -j $(nproc)
./avm code.asm
```

# About

## The language
the instructions set is declared in common.h
syntax is masm/nasm like
as example look into code.asm

## The project
### What is that
Week-long project, to prove the point.
No fancy design, KISS as main principle.
Code is definitely full of small bugs, there is no good testing.

### Motivation "Why?"
i have to work with emu8086 (old bullshit dos specific asm),
and i got so mad, so i made it as a prove of asm can be a lot better

## The architecture
### pipeline
code -> Lexer -> Parser -> VM executing
- **Lexer** accept the text and return a tokens
- **Parser** accept the tokens and return a Program (instructions; vars table)
- **VM** accepts the program and execute it

### Parsing
parser takes 3 passes to produce the program
1. Collect the vars
2. Collect the labels, skipping the ops, to get the right offset
3. parsing ops, filling the Program

vars does not depends on anything to parse;
labels depends on ops_count
ops depends on vars to parse (a little bit ambiguity)

