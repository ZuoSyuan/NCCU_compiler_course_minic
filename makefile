FILE=""
JFILE=$(FILE).j

minic: y.tab.c lex.yy.c symbol.o expr.o stmt.o parser.y lexer.l minic.h
	g++ -g y.tab.c lex.yy.c expr.o stmt.o symbol.o -o minic -lfl

stmt.o: stmt.h stmt.cpp
	g++ -g -c stmt.cpp

expr.o: expr.h expr.cpp
	g++ -g -c expr.cpp

symbol.o: symbol.h symbol.cpp
	g++ -g -c symbol.cpp
	
lex.yy.c: lexer.l y.tab.h
	lex lexer.l

y.tab.c y.tab.h: parser.y
	yacc -d parser.y

lexer_test: lex.yy.c lexer_test.o
	g++ -g lexer_test.o lex.yy.c -o lex_test -lfl

lexer_test.o: lexer_test.cpp
	g++ -c lexer_test.cpp
	
clean:
	-rm -rf *.o core.* *~ y.tab.c y.tab.h lex.yy.c

clobber:
	-rm -rf *.o core.* *~ y.tab.c y.tab.h lex.yy.c minic *.j 

run:
	./minic $(FILE) >$(JFILE)
	java -jar jasmin.jar $(JFILE)
	java $(FILE)
	
