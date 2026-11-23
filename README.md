# What is that

Week-long project, to prove the point.
No fancy design, KISS as main principle.
Code is definitely full of small bugs, there is no good testing.

# About the Architecture
 ## pipeline
 code -> Lexer -> Parser -> VM executing
 - **Lexer** accept the text and return a tokens
 - **Parser** accept the tokens and return a Program (instructions; vars table)
 - **VM** accepts the program and execute it

## Parsing
 parser takes 3 passes to produce the program
 1. Collect the vars
 2. Collect the labels, skipping the ops, to get the right offset
 3. parsing ops, filling the Program

 vars does not depends on anything to parse;
 labels depends on ops_count
 ops depends on vars to parse (a little bit ambiguity)

# About the language
the instructions set described in common.h
syntax is masm/nasm like

as example look into code.asm


