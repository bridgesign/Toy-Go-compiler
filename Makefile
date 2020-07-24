all: build clean

build: flex bison obj
	@echo "Building"
	gcc -lfl golang.tab.o stackframe.o regman.o blocks.o codeman.o expops.o lex.yy.o -o go

flex: tok.l
	@echo "Building Lexer..."
	flex tok.l

bison: golang.y
	@echo "Building Parser..."
	bison -d golang.y

obj: stackframe.c expops.c regman.c blocks.c codeman.c golang.tab.c lex.yy.c
	@echo "Compiling support files ..."
	gcc -c stackframe.c expops.c regman.c blocks.c codeman.c golang.tab.c lex.yy.c

clean:
	@echo "Cleaning..."
	rm lex.yy.c
	rm golang.tab.h
	rm golang.tab.c
	rm stackframe.o
	rm regman.o
	rm blocks.o
	rm codeman.o
	rm expops.o
	rm golang.tab.o
	rm lex.yy.o
