#ifndef MINIC_H
#define MINIC_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <string>

using namespace std;

class Expr;
class Stmt;
class SymbolTable;

enum Expr_Type { BOOL_TYPE=1, INT_TYPE, VOID_TYPE, GARBAGE_TYPE, 
								FLOAT_TYPE, BOOL_ARRAY_TYPE, INT_ARRAY_TYPE, FLOAT_ARRAY_TYPE};

enum Errortype { 	NOMAIN=1, 
									MAINHASARG,
									REDEFINEDFUN,
									REDEFINEDVAR,
									UNDEFINEDFUN,
									UNDEFINEDVAR,
									MISMATCHARG,
									ARGTYPEERR,
									RETTYPEERR,
									FUNCALLTYPEERR,
									IFSTMTERR,
									WLSTMTERR,
									FORSTMTERR,
									NOTARRERR,
									ARRINDERR,
									LVARERR,
									ASNTYPEERR,
									EXPRTYPEERR
									};
				
extern string filename;					
extern map<int, string> TypeTable;
extern map<int, string> TypeSignature;
extern FILE* yyin;
extern void printError(Errortype t, vector<string> info);
extern void checkMainFun();

extern int yylineno;
extern int yylex();
extern int yyparse(void);
extern void yyerror(string msg);
extern int currentLabel;
extern SymbolTable* currentSymbolTable;
extern Expr_Type currentReturnType;

extern int getLabel();
extern void emit(FILE* f, const char* op);
extern void emit(FILE* f, const char* op, int value);
extern void emit(FILE* f, const char* op, float value);
extern void emit(FILE* f, const char* op, const char* ident);
extern void emitJump(FILE* f, const char* op, int label);
extern void emitLabel(FILE* f, int label);
extern void emitGlobal(FILE* f, string ident, Expr_Type type);
#endif /* MINIC_H */
