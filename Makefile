all: format build test

format:
	@clang-format -style="{BasedOnStyle: Google,IndentWidth: 4,UseTab: Always,TabWidth: 4,ColumnLimit: 0}" -i *.h *.c

build:
	@gcc -std=c11 -Wall -Wextra -Wshadow -Werror -fmax-errors=1 -o testing -I. testing.c -ldl

test:
	@./testing
