all: bison flex gcc
	@echo "Done."

bison: parser.y
	bison parser.y

flex: scanner.l
	flex scanner.l

gcc: scanner.c parser.c types.c func_table.c str_table.c var_table.c ast.c gen.c
	gcc -Wall -o cc scanner.c parser.c types.c func_table.c str_table.c var_table.c ast.c gen.c

clean:
	@rm -rf cc scanner.c parser.h parser.c ./out/ ./trees/
