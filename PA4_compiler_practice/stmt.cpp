#include "stmt.h"
#include "symbol.h"

int Stmt::getStacksize() {
	return stacksize;
}

void Stmt::setStacksize(int size_) {
	stacksize = size_;
}

void ExprStmt::gencode(FILE *f) const {
	if(expr != NULL)
	{
		expr->gencode(f);
		if(expr->getType() != VOID_TYPE)
			emit(f, "pop");
	}
}

IfStmt::IfStmt(Expr* cond_, Stmt* then_) : cond(cond_), thenStmt(then_), elseStmt(NULL) {
	if (cond->getType() != BOOL_TYPE)
		printError(IFSTMTERR, vector<string>());
	setStacksize(max(cond->getStacksize(), thenStmt->getStacksize() ));
}

IfStmt::IfStmt(Expr* cond_, Stmt* then_, Stmt* else_) : cond(cond_), thenStmt(then_), elseStmt(else_) {
	if (cond->getType() != BOOL_TYPE)
		printError(IFSTMTERR, vector<string>());
	setStacksize(max(cond->getStacksize(), max(thenStmt->getStacksize(), elseStmt->getStacksize() )));
}

void IfStmt::gencode(FILE *f) const {

	int bottomLabel = getLabel();
	cond->gencode(f);
	if (elseStmt) {
		int middleLabel = getLabel();
		emitJump(f,"ifeq",middleLabel); // if false go to middle label (ELSE)
		thenStmt->gencode(f);
		emitJump(f,"goto",bottomLabel); // go to bottom label (EXIT)
		emitLabel(f, middleLabel);
		elseStmt->gencode(f);
	} else {
		emitJump(f,"ifeq",bottomLabel); // if false go to bottom label (EXIT)
		thenStmt->gencode(f);
	}
	emitLabel(f, bottomLabel); // emit label for bottom
}

WhileStmt::WhileStmt(Expr* cond_, Stmt* stmt_) : cond(cond_), stmt(stmt_) {
	if (cond->getType() != BOOL_TYPE)
		printError(WLSTMTERR, vector<string>());
	setStacksize(max(cond->getStacksize(), stmt->getStacksize() ));
}

void WhileStmt::gencode(FILE *f) const {

	int topLabel = getLabel();
	int bottomLabel = getLabel();
	emitLabel(f, topLabel); // emit label for top
	cond->gencode(f);
	emitJump(f,"ifeq",bottomLabel); // if false go to bottom label (EXIT)
	stmt->gencode(f);
	emitJump(f,"goto",topLabel); // go to top label (REPEAT)
	emitLabel(f, bottomLabel); // emit label for bottom
}


ReturnStmt::ReturnStmt(Expr* returnExpr_) : returnExpr(returnExpr_) {
	if(returnExpr != NULL)
	{
		if(returnExpr->getType() != currentReturnType)
			printError(RETTYPEERR, vector<string>());
		
		setStacksize(returnExpr->getStacksize() );
	}
}
/* TODO notice return instrusion with type or VOID */
void ReturnStmt::gencode(FILE *f) const {

}

CompoundStmt::CompoundStmt(vector<Expr* >* localDecls_, vector<Stmt* >* stmts_) : localDecls(localDecls_), stmts(stmts_)
{
	int size = 0;
	if(localDecls != NULL)
	{
		vector<Expr*>::const_iterator iter;
		for( iter = (*localDecls).begin(); iter != (*localDecls).end(); iter++ ) {
			size = max(size, (*iter)->getStacksize() );
		}
	}

	if(stmts != NULL)
	{
		vector<Stmt*>::const_iterator iter;
		for( iter = (*stmts).begin(); iter != (*stmts).end(); iter++ ) {
			size = max(size, (*iter)->getStacksize() );
		}
	}
	setStacksize(size);
}

void CompoundStmt::gencode(FILE *f) const {
	
	if(localDecls != NULL)
	{
		vector<Expr*>::const_iterator iter;
		for( iter = (*localDecls).begin(); iter != (*localDecls).end(); iter++ ) {
			(*iter)->gencode(f);
		}
	}

	if(stmts != NULL)
	{
		vector<Stmt*>::const_iterator iter;
		for( iter = (*stmts).begin(); iter != (*stmts).end(); iter++ ) {
			(*iter)->gencode(f);
		}
	}
}
FuncDeclStmt::FuncDeclStmt(Expr_Type returnType_, string ident_, vector<Expr_Type>* args_, Stmt* body_)
		: returnType(returnType_), ident(ident_), args(args_), body(body_) {
	if(!currentSymbolTable->getParent()->insertFunctionEntry(ident, args, returnType))
		printError(REDEFINEDFUN, vector<string>());
		
	if(body == NULL) yyerror("Function Declaration has null body");

	//subtract parameters off for local var count
	maxLocalVars = (currentSymbolTable->getMaxLocalCount());
	
	setStacksize(100);
}

void FuncDeclStmt::gencode(FILE *f) const {

	fprintf(f, ".method public static %s(", ident.c_str());

	//Emit a letter type for each argument
	if(args != NULL)
	{
		vector<Expr_Type>::const_iterator iter;
		for( iter = (*args).begin(); iter != (*args).end(); iter++ ) {
			fprintf(f, "%s", TypeSignature[(*iter)].c_str());
		}
	}
	
	// in JVM, main function hava a paramater string [].
	if(ident == "main")
	{
		fprintf(f, "[Ljava/lang/String;");
	}
	
	fprintf(f, ")");

	//Print return type
	fprintf(f, "%s", TypeSignature[returnType].c_str());

	//Print max number of local variables in scope at one time
	fprintf(f, "\n\t.limit locals %d\n", maxLocalVars);
	fprintf(f, "\t.limit stack %d\n", stacksize);
	
	//Print body of method
	if(body != NULL) body->gencode(f);

	//End method
	fprintf(f, ".end method\n\n");

}

StaticVarDeclStmt::StaticVarDeclStmt(Expr_Type type_, string ident_) : type(type_), ident(ident_) {
	if(!(currentSymbolTable->insertVariableEntry(ident, type)))
		printError(REDEFINEDVAR, vector<string>());
}

void StaticVarDeclStmt::gencode(FILE *f) const {
	emitGlobal(f, ident, type);
}
/* TODO count the stack size */
ForStmt::ForStmt(Expr* init_, Expr* cond_, Expr* variance_, Stmt* body_) : init(init_), cond(cond_), variance(variance_), body(body_)  {
	if (cond->getType() != BOOL_TYPE)
		printError(FORSTMTERR, vector<string>());
	
	
}
/* TODO */
void ForStmt::gencode(FILE *f) const {

}

StaticArrDeclStmt::StaticArrDeclStmt(Expr_Type type_, string ident_) : type(GARBAGE_TYPE), ident(ident_) {
	if(type_ == BOOL_TYPE) type = BOOL_ARRAY_TYPE;
	else if(type_ == INT_TYPE) type = INT_ARRAY_TYPE;
	else if(type_ == FLOAT_TYPE) type = FLOAT_ARRAY_TYPE;
	if(!(currentSymbolTable->insertVariableEntry(ident, type)))
		printError(REDEFINEDVAR, vector<string>());
}

/* TODO */
void StaticArrDeclStmt::gencode(FILE *f) const {
}

