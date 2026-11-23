CC = clang++ -Wall
CPPFLAGS = -Wall -Werror -std=c++23
LDFLAGS = 

NAME=avm

#CPPFLAGS = -Wall -std=c++23 -g -O1 -fsanitize=address -fno-omit-frame-pointer
#LDFLAGS = -fsanitize=address

all: build 
build: main

obj:
	mkdir -p $@

main: obj/main.o obj/lexer.o obj/utils.o obj/parser.o obj/vm.o
	$(CC) $(LDFLAGS) -o $(NAME) $^

main.cpp: lexer.h
obj/main.o: main.cpp | obj
	$(CC) $(CPPFLAGS) -o $@ -c $<
	
lexer.h: common.h
lexer.cpp: lexer.h
obj/lexer.o: lexer.cpp lexer.h | obj
	$(CC) $(CPPFLAGS) -o $@ -c $<

utils.h: common.h
utils.cpp: utils.h
obj/utils.o: utils.cpp | obj
	$(CC) $(CPPFLAGS) -o $@ -c $<

parser.h: common.h
parser.cpp: parser.h
obj/parser.o: parser.cpp | obj
	$(CC) $(CPPFLAGS) -o $@ -c $<

vm.h: common.h
vm.cpp: vm.h
obj/vm.o: vm.cpp | obj
	$(CC) $(CPPFLAGS) -o $@ -c $<

run:
	./main

clean:
	rm -rf obj
	rm -f main

.PHONY: build run clean
