%{

#include "minic.h"
#include "expr.h"
#include "stmt.h"
#include "symbol.h"

string filename;
map<int, string> TypeTable;
map<int, string> TypeSignature;
int currentLabel = 0;;
SymbolTable* currentSymbolTable = new SymbolTable(NULL);
Expr_Type currentReturnType = GARBAGE_TYPE;

vector<Stmt*>* AST;

int getLabel()
{
	int returnLabel = currentLabel;
	currentLabel++;
	return returnLabel;
}

void emitchars(FILE* f, const char* op)
{
	fprintf(f, "%s", op);
}

void emit(FILE* f, const char* op)
{
	fprintf(f, "\t%s\n", op);
}

void emit(FILE* f, const char* op, int value)
{
	fprintf(f, "\t%s %d\n", op, value);
}

void emit(FILE* f, const char* op, float value)
{
	fprintf(f, "\t%s %f\n", op, value);
}

void emit(FILE* f, const char* op, const char* ident)
{
	fprintf(f, "\t%s %s\n", op, ident);
}

void emitGlobal(FILE* f, string ident, Expr_Type type)
{
	if(TypeSignature.find(type) != TypeSignature.end())
		fprintf(f, ".field public static %s %s\n\n", ident.c_str(), TypeSignature[type].c_str());
	else
		yyerror("emitGlobal can not be performed on this type!");
	/*
	if(type == INT_TYPE) fprintf(f, ".field public static %s I\n\n", ident.c_str());
	else if(type == BOOL_TYPE) fprintf(f, ".field public static %s Z\n\n", ident.c_str());
	else if(type == FLOAT_TYPE) fprintf(f, ".field public static %s F\n\n", ident.c_str());
	else yyerror("emitGlobal can not be performed on this type!");
	*/
}

void emitJump(FILE* f, const char* op, int label)
{
	fprintf(f, "\t%s #_%d\n", op, label);
}

void emitLabel(FILE* f, int label)
{
	fprintf(f, "#_%d:\n", label);
}

void programGencode(FILE* f, vector<Stmt*>* stmtList)
{
	emitchars(f, "\n.class public ");
	emitchars(f, filename.c_str());
	emitchars(f, "\n.super java/lang/Object\n\n");
	/*
	emitchars(f, ".method public <init>()V\n");
	emitchars(f, "\taload_0\n");
	emitchars(f, "\tinvokespecial java/lang/Object/<init>()V\n");
	emitchars(f, "\treturn\n");
	emitchars(f, ".end method\n\n");
	*/
	
	
	vector<Stmt*>::const_iterator iter;
	for( iter = (*stmtList).begin(); iter != (*stmtList).end(); iter++ ) {
		(*iter)->gencode(f);
	}
}

void newScope()
{
	currentSymbolTable = new SymbolTable(currentSymbolTable);
}

void endScope()
{
	if(currentSymbolTable->getParent() != NULL)
	{
		//Set the maximum used index of the parent to the child if its greater
		currentSymbolTable->getParent()->setMaxLocalCount(currentSymbolTable->getMaxLocalCount());

		//restore scope to parent
		currentSymbolTable = currentSymbolTable->getParent(); 
	}
	else yyerror("Tried to return scope with no parent");
}

%}

%union {
  char *s;
  int i;
  bool b;
  float f;
  Expr_Type type;
  Expr* expr;
  Stmt* stmt;
  vector<Expr_Type>* typeList;
  vector<Stmt*>* stmtList;
  vector<Expr*>* exprList;
}

%expect 1 /* dangling else problem auto shifted */

%token <s> IDENT
%token IF ELSE WHILE RETURN NEW SIZE FOR
%token OR AND
%token VOID_TOKEN BOOL_TOKEN INT_TOKEN FLOAT_TOKEN
%token LE '<' '>' GE EQ NE
%token <i> INT_LITERAL
%token <b> BOOL_LITERAL
%token <f> FLOAT_LITERAL
%token '[' ']' '.'

%type <expr> expr local_decl local_var_decl local_array_decl lval
%type <stmtList> decl_list stmt_list
%type <exprList> arg_list args local_decls
%type <type> param
%type <typeList> param_list params
%type <stmt> decl stmt var_decl fun_decl expr_stmt compound_stmt if_stmt while_stmt return_stmt for_stmt array_decl
%type <type> type_spec

%right '='
%left OR
%left AND
%nonassoc LE '<' '>' GE EQ NE
%left '+' '-'
%left '*' '/' '%'
%right UNARY_OP


%%

program 		: decl_list				{ checkMainFun(); AST = $1; }
			;

decl_list 		: decl_list decl			{ $$ = $1; $$->push_back($2); }
			| decl					{ $$ = new vector<Stmt*>; $$->push_back($1); }
			;

decl			: var_decl				{ $$ = $1; }
			| fun_decl				{ $$ = $1; }
			| array_decl			{ $$ = $1; }
			;

var_decl		: type_spec IDENT ';'		{ $$ = new StaticVarDeclStmt($1, $2); }
			;

type_spec		: VOID_TOKEN				{ $$ = VOID_TYPE; }
			| BOOL_TOKEN				{ $$ = BOOL_TYPE; }
			| INT_TOKEN				{ $$ = INT_TYPE; }
			| FLOAT_TOKEN				{ $$ = FLOAT_TYPE; }
			;

fun_decl		: type_spec IDENT '(' 
				{ newScope(); currentReturnType = $1; }
				params 
				{ // in JVM, main function hava a paramater string []. 
				if(string($2) == "main") currentSymbolTable->insertVariableEntry("MAIN_PARAMATER", GARBAGE_TYPE); }
				')' compound_stmt
				{ $$ = new FuncDeclStmt($1, $2, $5, $8); endScope(); currentReturnType = GARBAGE_TYPE;}
			;

array_decl		: type_spec '[' ']' IDENT ';' 	{ $$ = new StaticArrDeclStmt($1, $4); }
			;
							
params			: param_list				{ $$ = $1; }
			| VOID_TOKEN				{ $$ = new vector<Expr_Type>; }
			;

param_list		: param_list ',' param		{ $$ = $1; $$->push_back($3); }
			| param				{ $$ = new vector<Expr_Type>; $$->push_back($1); }
			;

param			: type_spec IDENT			{ $$ = $1; currentSymbolTable->insertVariableEntry($2, $1);}
			;

compound_stmt		: 	'{' { newScope(); }
			local_decls stmt_list '}' { $$ = new CompoundStmt($3, $4); endScope(); }
			;

local_decls		: local_decls local_decl		{ $$ = $1; $$->push_back($2); }
			|					{ $$ = new vector<Expr*>; }
			;

local_decl	: local_var_decl { $$ = $1; }
			| local_array_decl { $$ = $1; }
			;
			
local_var_decl		: type_spec IDENT ';'		{ $$ = new LocalDeclExpr($2, $1); }
			;
			
local_array_decl		: type_spec '[' ']' IDENT ';'		{ $$ = new ArrayDeclExpr($4, $1); }
			;
			
stmt_list		: stmt_list stmt			{ $$ = $1; $$->push_back($2); }
			|					{ $$ = new vector<Stmt*>; }
			;

stmt			: expr_stmt				{ $$ = $1; }
			| compound_stmt			{ $$ = $1; }
			| if_stmt				{ $$ = $1; }
			| while_stmt				{ $$ = $1; }
			| return_stmt				{ $$ = $1; }
			| for_stmt 					{$$ = $1;}
			;

expr_stmt		: expr ';'				{ $$ = new ExprStmt($1); }
			| ';'					{ $$ = new NullStmt(); }
			;

while_stmt		: WHILE '(' expr ')' stmt		{ $$ = new WhileStmt($3, $5); }
			;

if_stmt		: IF '(' expr ')' stmt		{ $$ = new IfStmt($3, $5, NULL); }
			| IF '(' expr ')' stmt ELSE stmt	{ $$ = new IfStmt($3, $5, $7); }
			;

return_stmt		: RETURN ';'				{ $$ = new ReturnStmt(NULL); }
			| RETURN expr ';'			{ $$ = new ReturnStmt($2); }
			;

for_stmt 	: FOR '(' expr ';' expr ';' expr ')' stmt 	{ $$ = new ForStmt($3, $5, $7, $9); }
			;
			
expr			: lval '=' expr			{ $$ = new VarAssignmentExpr($1, $3); }
			| expr OR expr			{ $$ = new OrBinaryExpr($1, $3); }
			| expr AND expr			{ $$ = new AndBinaryExpr($1, $3); }
			| expr EQ expr			{ $$ = new EQCompareBinaryExpr($1, $3); }
			| expr NE expr			{ $$ = new NECompareBinaryExpr($1, $3); }
			| expr LE expr			{ $$ = new LECompareBinaryExpr($1, $3); }
			| expr '<' expr			{ $$ = new LTCompareBinaryExpr($1, $3); }
			| expr GE expr			{ $$ = new GECompareBinaryExpr($1, $3); }
			| expr '>' expr			{ $$ = new GTCompareBinaryExpr($1, $3); }
			| expr '+' expr			{ $$ = new AdditionBinaryExpr($1, $3); }
			| expr '-' expr			{ $$ = new SubtractionBinaryExpr($1, $3); }
			| expr '*' expr			{ $$ = new MultiplicationBinaryExpr($1, $3); }
			| expr '/' expr			{ $$ = new DivisionBinaryExpr($1, $3); }
			| expr '%' expr			{ $$ = new ModulusBinaryExpr($1, $3); }
			| '!' expr	%prec UNARY_OP	{ $$ = new NegateUnaryExpr($2); }
			| '+' expr	%prec UNARY_OP	{ $$ = new AdditionUnaryExpr($2); }
			| '-' expr	%prec UNARY_OP	{ $$ = new SubtractUnaryExpr($2); }
			| '(' expr ')'			{ $$ = $2; }
			| IDENT				{ $$ = new IdentExpr($1); }
			| IDENT '(' args ')'			{ $$ = new CallExpr($1, $3); }
			| BOOL_LITERAL			{ $$ = new BoolConstExpr($1); }
			| INT_LITERAL				{ $$ = new IntConstExpr($1); }
			| FLOAT_LITERAL				{$$ = new FloatConstExpr($1); }
			| IDENT '[' expr ']' 	{ $$ = new IdentArrayExpr($1, $3); }
			| NEW type_spec '[' expr ']'		{$$ = new NewArrayExpr($2, $4);}
			| IDENT '.' SIZE { $$ = new GetArraySizeExpr($1);}
			;

lval	:	IDENT	{ $$ = new LeftValueExpr($1, NULL); }
			|	IDENT '[' expr ']' { $$ = new LeftValueExpr($1, $3); }
			;

arg_list		: arg_list ',' expr	{ $$ = $1; $$->push_back($3); }
			| expr			{ $$ = new vector<Expr*>; $$->push_back($1); }
			;

args			: arg_list		{ $$ = $1; }
			|			{ $$ = new vector<Expr*>; }
			;

%%

void printError(Errortype t, vector<string> info)
{
	switch(t)
	{
		case NOMAIN:
			printf("[%d]: no main function\n", yylineno);
			break; 
		case MAINHASARG:
			printf("[%d]: main function has arguments\n", yylineno);
			break; 
		case REDEFINEDFUN:
			printf("[%d]: Duplicate Functions\n", yylineno);
			break; 
		case REDEFINEDVAR:
			printf("[%d]: Duplicate Variables\n", yylineno);
			break; 
		case UNDEFINEDFUN:
			printf("[%d]: Undefined Functions\n", yylineno);
			break; 
		case UNDEFINEDVAR:
			printf("[%d]: Undefined Variables\n", yylineno);
			break; 
		case MISMATCHARG:
			printf("[%d]: Number of argument mismatch\n", yylineno);
			break; 
		case ARGTYPEERR:
			printf("[%d]: Argument type error\n", yylineno);
			break; 
		case RETTYPEERR:
			printf("[%d]: Return type error\n", yylineno);
			break; 
		case FUNCALLTYPEERR:
			printf("[%d]: Function call assignment type error\n", yylineno);
			break; 
		case IFSTMTERR:
			printf("[%d]: If statement premise type error\n", yylineno);
			break; 
		case WLSTMTERR:
			printf("[%d]: While loop requirement type error\n", yylineno);
			break; 
		case FORSTMTERR:
			printf("[%d]: For loop requirement type error\n", yylineno);
			break; 
		case NOTARRERR:
			printf("[%d]: Not an array error\n", yylineno);
			break; 
		case ARRINDERR:
			printf("[%d]: Array index type error\n", yylineno);
			break; 
		case LVARERR:
			printf("[%d]: Wrong l_value\n", yylineno);
			break; 
		case ASNTYPEERR:
			printf("[%d]: Assignment type error\n", yylineno);
			break; 
		case EXPRTYPEERR:
			printf("[%d]: Expression type error\n", yylineno);
			break; 
		default:
			yyerror("The compiler process occurs an analysis error on invoking printError()");
	}
}

void checkMainFun()
{
	if( !currentSymbolTable->getFunctionEntry("main") )
		printError(NOMAIN, vector<string>());
	else if( !currentSymbolTable->getFunctionEntry("main")->getArgs()->empty() )
		printError(MAINHASARG, vector<string>());
}

void yyerror(string msg) 
{
  fprintf(stderr, "[%d] : %s\n", yylineno, msg.c_str());
  exit(-1);
}

void initTypeTab()
{
	TypeTable[BOOL_TYPE] = string("bool");
	TypeTable[INT_TYPE] = string("int");
	TypeTable[VOID_TYPE] = string("void");
	TypeTable[GARBAGE_TYPE] = string("garbage");
	TypeTable[FLOAT_TYPE] = string("float");
	TypeTable[BOOL_ARRAY_TYPE] = string("bool[]");
	TypeTable[INT_ARRAY_TYPE] = string("int[]");
	TypeTable[FLOAT_ARRAY_TYPE] = string("float[]");
}

void initTypeSig()
{
	TypeSignature[BOOL_TYPE] = string("Z");
	TypeSignature[INT_TYPE] = string("I");
	TypeSignature[VOID_TYPE] = string("V");
	TypeSignature[GARBAGE_TYPE] = string("X");
	TypeSignature[FLOAT_TYPE] = string("F");
	TypeSignature[BOOL_ARRAY_TYPE] = string("[Z");
	TypeSignature[INT_ARRAY_TYPE] = string("[I");
	TypeSignature[FLOAT_ARRAY_TYPE] = string("[F");
}


void initIOfunctions()
{
	vector<Expr_Type>* ireadArgs = new vector<Expr_Type>;
	currentSymbolTable->insertFunctionEntry("iread", ireadArgs, INT_TYPE);
	
	vector<Expr_Type>* freadArgs = new vector<Expr_Type>;
	currentSymbolTable->insertFunctionEntry("fread", ireadArgs, FLOAT_TYPE);
	
	vector<Expr_Type>* iprintArgs = new vector<Expr_Type>;
	iprintArgs->push_back(INT_TYPE);
	currentSymbolTable->insertFunctionEntry("iprint", iprintArgs, VOID_TYPE);
		
	vector<Expr_Type>* fprintArgs = new vector<Expr_Type>;
	fprintArgs->push_back(FLOAT_TYPE);
	currentSymbolTable->insertFunctionEntry("fprint", fprintArgs, VOID_TYPE);
	
}

int main(int argc, char *argv[]) 
{

	FILE *file;
	if (argc < 2) {fprintf(stderr, "Usage: minic [filename]"); return 1;}
	file = fopen(argv[1], "r");
	if (!file) {fprintf(stderr, "Unable to open file %s", argv[1]); return 1;}
	yyin = file;
	filename = string(argv[1]);
	
	//init
	initTypeTab();
	initTypeSig();
	//Library functions
	initIOfunctions();
	
	//Run the parse
	yyparse();

	programGencode(stdout, AST);

	//Looks like all is well
	return 0;
}
