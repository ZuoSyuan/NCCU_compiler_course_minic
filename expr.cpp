#include "expr.h"
#include "symbol.h"


Expr_Type Expr::getType() const {
	return type;
}

void Expr::setType(Expr_Type type_) {
	type = type_;
}


void IntConstExpr::gencode(FILE* f) const {
	emit(f, "ldc", value);
}

void BoolConstExpr::gencode(FILE* f) const {
	emit(f, "ldc", ((value) ? 1 : 0));
}

void FloatConstExpr::gencode(FILE* f) const {
	emit(f, "ldc", value);
}

CallExpr::CallExpr(string ident_, vector<Expr*>* args_) : Expr(GARBAGE_TYPE), ident(ident_), args(args_) {
	FunctionEntry* currentFunction = currentSymbolTable->getFunctionEntry(ident);
	if(currentFunction == NULL) 
	{
		printError(UNDEFINEDFUN, vector<string>());
		return;
	}
	type = currentFunction->getType();	
	/* LOOK UP FUNCTION ARGS AND COMPARE TO CALLED ARGS*/	
	vector<Expr_Type>* argList = currentFunction->getArgs();

	if(argList == NULL || args == NULL)
		yyerror("Null arguments to function");

	vector<Expr_Type>::iterator funcIter = (*argList).begin();
	vector<Expr*>::iterator callIter = (*args).begin();

	if((*argList).size() != (*args).size()) 
	{
		printError(MISMATCHARG, vector<string>());
		return;
	}
	while(funcIter != (*argList).end() && callIter != (*args).end() )
	{
		if((*callIter)->getType() != (*funcIter))
		{
			printError(ARGTYPEERR, vector<string>());
		}

		funcIter++;
		callIter++;
	}
	
}

void CallExpr::gencode(FILE* f) const {
	vector<Expr*>::const_iterator iter;
	for( iter = (*args).begin(); iter != (*args).end(); iter++ ) {
		(*iter)->gencode(f);
	}
	
	if(ident == "iread")
		emit(f, "invokestatic", "IOclass/iread()I");
	else if(ident == "fread")
		emit(f, "invokestatic", "IOclass/fread()F");
	else if(ident == "iprint")
		emit(f, "invokestatic", "IOclass/iprint(I)V");
	else if(ident == "fprint")
		emit(f, "invokestatic", "IOclass/fprint(F)V");
	else
	{
		fprintf(f, "\tinvokestatic %s/%s(", filename.c_str(), ident.c_str());	
		//Emit a letter type for each argument
		if(args != NULL)
		{
			vector<Expr*>::const_iterator iter;
			for( iter = (*args).begin(); iter != (*args).end(); iter++ ) {
				fprintf(f, "%s", TypeSignature[(*iter)->getType()].c_str());
				/*
				if((*iter)->getType() == BOOL_TYPE) fprintf(f, "Z");
				else if((*iter)->getType() == INT_TYPE) fprintf(f, "I");
				else if((*iter)->getType() == FLOAT_TYPE) fprintf(f, "F");
				*/
			}
		}
		fprintf(f, ")");
		fprintf(f, "%s", TypeSignature[type].c_str());
		/*
		if(type == BOOL_TYPE) fprintf(f, "Z");
		else if(type == INT_TYPE) fprintf(f, "I");
		else if(type == FLOAT_TYPE) fprintf(f, "F");
		*/
		fprintf(f, "\n");
	}
}

LocalDeclExpr::LocalDeclExpr(string ident_, Expr_Type type_) : Expr(type_), ident(ident_) {
	if(!(currentSymbolTable->insertVariableEntry(ident, type)))
	{
		printError(REDEFINEDVAR, vector<string>());
		return;
	}
	//if there are previous entries of this variable give a warning
	if(currentSymbolTable->getParent()->getVariableEntry(ident) != NULL)
		fprintf(stderr, "[%d] : WARNING: Variable shadows a previous variable\n", yylineno);

	// symbol table increments offset after setting so we need to subtract 1
	offset = currentSymbolTable->getOffset() - 1;
}

void LocalDeclExpr::gencode(FILE* f) const {
	if(type == FLOAT_TYPE)
	{
		emit(f, "ldc 0.0");
		emit(f, "fstore", offset);
	}
	else
	{
		emit(f, "iconst_0");
		emit(f, "istore", offset);
	}
}

///
ArrayDeclExpr::ArrayDeclExpr(string ident_, Expr_Type type_) : Expr(GARBAGE_TYPE), ident(ident_) {
	if(type_ == BOOL_TYPE) setType(BOOL_ARRAY_TYPE);
	else if(type_ == INT_TYPE) setType(INT_ARRAY_TYPE);
	else if(type_ == FLOAT_TYPE) setType(FLOAT_ARRAY_TYPE);
	if(!(currentSymbolTable->insertVariableEntry(ident, type)))
		printError(REDEFINEDVAR, vector<string>());
		
	//if there are previous entries of this variable give a warning
	if(currentSymbolTable->getParent()->getVariableEntry(ident) != NULL)
		fprintf(stderr, "[%d] : WARNING: Variable shadows a previous variable\n", yylineno);
		
	// symbol table increments offset after setting so we need to subtract 1
	offset = currentSymbolTable->getOffset() - 1;
}

void ArrayDeclExpr::gencode(FILE* f) const {
}

///
NewArrayExpr::NewArrayExpr(Expr_Type type_, Expr* indexexpr_) : Expr(GARBAGE_TYPE), indexexpr(indexexpr_) {	
	if(indexexpr && indexexpr->getType() != INT_TYPE) 
		printError(ARRINDERR, vector<string>());

	if(type_ == BOOL_TYPE) setType(BOOL_ARRAY_TYPE);
	else if(type_ == INT_TYPE) setType(INT_ARRAY_TYPE);
	else if(type_ == FLOAT_TYPE) setType(FLOAT_ARRAY_TYPE);
}

void NewArrayExpr::gencode(FILE* f) const {
	indexexpr->gencode(f);
	
	string s;
	if(type == BOOL_ARRAY_TYPE) s = "bool";
	else if(type == INT_ARRAY_TYPE) s = "int";
	else if(type == FLOAT_ARRAY_TYPE) s = "float";
	emit(f, "newarray", s.c_str());
}

///
GetArraySizeExpr::GetArraySizeExpr(string ident_) : Expr(INT_TYPE), ident(ident_) 
{
	VariableEntry* currentVariable = currentSymbolTable->getVariableEntry(ident);
	if(currentVariable == NULL) { printError(UNDEFINEDVAR, vector<string>()); return;}

	index = currentVariable->getIndex();
	isGlobal = currentVariable->isGlobal();
	oritype = currentVariable->getType();
	
	if(oritype != BOOL_ARRAY_TYPE && oritype != INT_ARRAY_TYPE && oritype != FLOAT_ARRAY_TYPE)
		printError(NOTARRERR, vector<string>());
}

void GetArraySizeExpr::gencode(FILE* f) const {
	if(isGlobal) 
	{
			string s = filename + "/" + ident;
			s += " " + TypeSignature[oritype];
			emit(f, "getstatic", s.c_str());
	}
	else 
		emit(f, "aload", index);

	emit(f, "arraylength");
}

///IdentArrayExpr
IdentArrayExpr::IdentArrayExpr(string ident_, Expr* indexexpr_) : Expr(GARBAGE_TYPE), ident(ident_), indexexpr(indexexpr_) 
{
	VariableEntry* currentVariable = currentSymbolTable->getVariableEntry(ident);
	if(currentVariable == NULL) { printError(UNDEFINEDVAR, vector<string>()); return;}
	
	index = currentVariable->getIndex();
	isGlobal = currentVariable->isGlobal();
	oritype = currentVariable->getType();
	
	if(currentVariable->getType() == INT_ARRAY_TYPE) type = INT_TYPE;
	else if(currentVariable->getType() == BOOL_ARRAY_TYPE) type = BOOL_TYPE;
	else if(currentVariable->getType() == FLOAT_ARRAY_TYPE) type = FLOAT_TYPE;
	else type = currentVariable->getType();

	if(indexexpr && indexexpr->getType() != INT_TYPE) 
		printError(ARRINDERR, vector<string>());
}

void IdentArrayExpr::gencode(FILE* f) const {
	if(isGlobal) 
	{
			string s = filename + "/" + ident;
			s += " " + TypeSignature[oritype];
			emit(f, "getstatic", s.c_str());
	}
	else 
		emit(f, "aload", index);
	
	indexexpr->gencode(f);
	
	if(type == FLOAT_TYPE)
		emit(f, "faload");
	else
		emit(f, "iaload");
}

IdentExpr::IdentExpr(string ident_) : Expr(GARBAGE_TYPE), ident(ident_) {
	VariableEntry* currentVariable = currentSymbolTable->getVariableEntry(ident);
	if(currentVariable == NULL) { printError(UNDEFINEDVAR, vector<string>()); return;}
	type = currentVariable->getType();
	index = currentVariable->getIndex();
	isGlobal = currentVariable->isGlobal();
}

void IdentExpr::gencode(FILE* f) const {
	if(isGlobal) // emit(f, "getstatic", ident.c_str());
	{
		string s = filename + "/" + ident;
		s += " " + TypeSignature[type];
		emit(f, "getstatic", s.c_str());
	}
	else 
	{
		if(type == FLOAT_TYPE)
			emit(f, "fload", index);
		else
			emit(f, "iload", index);
	}
}

///LeftValueExpr
LeftValueExpr::LeftValueExpr(string ident_, Expr* indexexpr_) : Expr(GARBAGE_TYPE), ident(ident_), indexexpr(indexexpr_) 
{
	VariableEntry* currentVariable = currentSymbolTable->getVariableEntry(ident);
	if(currentVariable == NULL)
	{ 
		printError(UNDEFINEDVAR, vector<string>()); 
		return;
	}
	index = currentVariable->getIndex();
	isGlobal = currentVariable->isGlobal();
	oritype = currentVariable->getType();
	
	// it's an array element
	if(indexexpr)
	{
		if(currentVariable->getType() == INT_ARRAY_TYPE) type = INT_TYPE;
		else if(currentVariable->getType() == BOOL_ARRAY_TYPE) type = BOOL_TYPE;
		else if(currentVariable->getType() == FLOAT_ARRAY_TYPE) type = FLOAT_TYPE;
	}
	else
		type = currentVariable->getType();
		
	if(indexexpr && indexexpr->getType() != INT_TYPE) 
		printError(ARRINDERR, vector<string>());

}

void LeftValueExpr::gencode(FILE* f) const {
	if(isGlobal) 
	{
			string s = filename + "/" + ident;
			s += " " + TypeSignature[oritype];
			emit(f, "getstatic", s.c_str());
	}
	else 
		emit(f, "aload", index);
	indexexpr->gencode(f);
}

VarAssignmentExpr::VarAssignmentExpr(Expr* left_, Expr* right_) : Expr(GARBAGE_TYPE), left(left_), right(right_) {
	
	if(left->getType() != right->getType())
		printError(ASNTYPEERR, vector<string>());

	setType(left->getType());
}

void VarAssignmentExpr::gencode(FILE* f) const {
	//load Lval
	if(((LeftValueExpr*)left)->indexexpr)
		left->gencode(f);
	
	// count Rval
	right->gencode(f);
	
	// keep the expr value in stack
	if(((LeftValueExpr*)left)->indexexpr)
		emit(f, "dup_x2");	
	else
		emit(f, "dup");	
	
	// store in Lval
	if(((LeftValueExpr*)left)->indexexpr)
	{
		if(type == FLOAT_TYPE)
			emit(f, "fastore");
		else
			emit(f, "iastore");
	}
	else if(((LeftValueExpr*)left)->isGlobal) 
	{
		string s = filename + "/" + ((LeftValueExpr*)left)->ident;
		s += " " + TypeSignature[((LeftValueExpr*)left)->oritype];
		emit(f, "putstatic", s.c_str());
	}
	else
	{ 
		if(type == INT_ARRAY_TYPE || type == BOOL_ARRAY_TYPE || type == FLOAT_ARRAY_TYPE)
			emit(f, "astore", ((LeftValueExpr*)left)->index);
		else if(type == FLOAT_TYPE)
			emit(f, "fstore", ((LeftValueExpr*)left)->index);
		else
			emit(f, "istore", ((LeftValueExpr*)left)->index);
	}
}

NegateUnaryExpr::NegateUnaryExpr(Expr* right_) : UnaryExpr(right_) {
	if(right->getType() != BOOL_TYPE)
		printError(EXPRTYPEERR, vector<string>());
	
	setType(BOOL_TYPE);
}

void NegateUnaryExpr::gencode(FILE* f) const {
	right->gencode(f);
	emit(f, "iconst_1");
	emit(f, "ixor");
}

AdditionUnaryExpr::AdditionUnaryExpr(Expr* right_) : UnaryExpr(right_) {
	if(right->getType() == BOOL_TYPE)
		printError(EXPRTYPEERR, vector<string>());

	setType(INT_TYPE);
}

void AdditionUnaryExpr::gencode(FILE* f) const {
	right->gencode(f);
}

SubtractUnaryExpr::SubtractUnaryExpr(Expr* right_) : UnaryExpr(right_)
{
	if(right->getType() == BOOL_TYPE)
		printError(EXPRTYPEERR, vector<string>());

	setType(INT_TYPE);
}

void SubtractUnaryExpr::gencode(FILE* f) const {
	right->gencode(f);
	emit(f, "ineg");
}






NumberBinaryExpr::NumberBinaryExpr(Expr* left_, Expr* right_) : BinaryExpr(left_, right_) {
	if(left->getType() == BOOL_TYPE || right->getType() == BOOL_TYPE )
		printError(EXPRTYPEERR, vector<string>());
	
	if(left->getType() == FLOAT_TYPE || right->getType() == FLOAT_TYPE)
	{
		left_i2f = ( left->getType() == INT_TYPE );
		right_i2f = ( right->getType() == INT_TYPE );
		setType(FLOAT_TYPE);
	}
	else
	{
		left_i2f = right_i2f = false;
		setType(INT_TYPE);
	}
}


void AdditionBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	if(left_i2f) emit(f, "i2f");
	right->gencode(f);
	if(right_i2f) emit(f, "i2f");
	
	if(type == FLOAT_TYPE)
		emit(f, "fadd");
	else
		emit(f, "iadd");
}

void SubtractionBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	if(left_i2f) emit(f, "i2f");
	right->gencode(f);
	if(right_i2f) emit(f, "i2f");
	
	if(type == FLOAT_TYPE)
		emit(f, "fsub");
	else
		emit(f, "isub");
}

void MultiplicationBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	if(left_i2f) emit(f, "i2f");
	right->gencode(f);
	if(right_i2f) emit(f, "i2f");
	
	if(type == FLOAT_TYPE)
		emit(f, "fmul");
	else
		emit(f, "imul");
}

void DivisionBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	if(left_i2f) emit(f, "i2f");
	right->gencode(f);
	if(right_i2f) emit(f, "i2f");
	
	if(type == FLOAT_TYPE)
		emit(f, "fdiv");
	else
		emit(f, "idiv");
}

void ModulusBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	if(left_i2f) emit(f, "i2f");
	right->gencode(f);
	if(right_i2f) emit(f, "i2f");
	
	if(type == FLOAT_TYPE)
		emit(f, "frem");
	else
		emit(f, "irem");
}



LogicBinaryExpr::LogicBinaryExpr(Expr* left_, Expr* right_) : BinaryExpr(left_, right_) {
	if(left->getType() != BOOL_TYPE || right->getType() != BOOL_TYPE )
		printError(EXPRTYPEERR, vector<string>());

	setType(BOOL_TYPE);
}


void OrBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	right->gencode(f);
	emit(f, "ior");
}

void AndBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	right->gencode(f);
	emit(f, "iand");
}


CompareBinaryExpr::CompareBinaryExpr(Expr* left_, Expr* right_) : BinaryExpr(left_, right_) { 
	type = BOOL_TYPE;
}

LECompareBinaryExpr::LECompareBinaryExpr(Expr* left_, Expr* right_) : CompareBinaryExpr(left_, right_) {
	if(left->getType() == BOOL_TYPE || right->getType() == BOOL_TYPE)
		printError(EXPRTYPEERR, vector<string>());
}

LTCompareBinaryExpr::LTCompareBinaryExpr(Expr* left_, Expr* right_) : CompareBinaryExpr(left_, right_) {
	if(left->getType() == BOOL_TYPE || right->getType() == BOOL_TYPE)
		printError(EXPRTYPEERR, vector<string>());
}

GECompareBinaryExpr::GECompareBinaryExpr(Expr* left_, Expr* right_) : CompareBinaryExpr(left_, right_) {
	if(left->getType() == BOOL_TYPE || right->getType() == BOOL_TYPE)
		printError(EXPRTYPEERR, vector<string>());
}

GTCompareBinaryExpr::GTCompareBinaryExpr(Expr* left_, Expr* right_) : CompareBinaryExpr(left_, right_) {
	if(left->getType() == BOOL_TYPE || right->getType() == BOOL_TYPE)
		printError(EXPRTYPEERR, vector<string>());
}

EQCompareBinaryExpr::EQCompareBinaryExpr(Expr* left_, Expr* right_) : CompareBinaryExpr(left_, right_) {
	if(left->getType() != right->getType())
		printError(EXPRTYPEERR, vector<string>());
}

NECompareBinaryExpr::NECompareBinaryExpr(Expr* left_, Expr* right_) : CompareBinaryExpr(left_, right_) {
	if(left->getType() != right->getType())
		printError(EXPRTYPEERR, vector<string>());
}


void LECompareBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	right->gencode(f);

	int label1 = getLabel();
	int label2 = getLabel();

	emit(f, "isub");
	emitJump(f, "ifle", label1);
	emit(f, "ldc", 0);
	emitJump(f, "goto", label2);
	emitLabel(f, label1);
	emit(f, "ldc", 1);
	emitLabel(f, label2);
}

void LTCompareBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	right->gencode(f);

	int label1 = getLabel();
	int label2 = getLabel();

	emit(f, "isub");
	emitJump(f, "iflt", label1);
	emit(f, "ldc", 0);
	emitJump(f, "goto", label2);
	emitLabel(f, label1);
	emit(f, "ldc", 1);
	emitLabel(f, label2);
}

void GECompareBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	right->gencode(f);

	int label1 = getLabel();
	int label2 = getLabel();

	emit(f, "isub");
	emitJump(f, "ifge", label1);
	emit(f, "ldc", 0);
	emitJump(f, "goto", label2);
	emitLabel(f, label1);
	emit(f, "ldc", 1);
	emitLabel(f, label2);
}

void GTCompareBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	right->gencode(f);

	int label1 = getLabel();
	int label2 = getLabel();

	emit(f, "isub");
	emitJump(f, "ifgt", label1);
	emit(f, "ldc", 0);
	emitJump(f, "goto", label2);
	emitLabel(f, label1);
	emit(f, "ldc", 1);
	emitLabel(f, label2);
}

void EQCompareBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	right->gencode(f);

	int label1 = getLabel();
	int label2 = getLabel();

	emit(f, "isub");
	emitJump(f, "ifeq", label1);
	emit(f, "ldc", 0);
	emitJump(f, "goto", label2);
	emitLabel(f, label1);
	emit(f, "ldc", 1);
	emitLabel(f, label2);
}

void NECompareBinaryExpr::gencode(FILE* f) const {
	left->gencode(f);
	right->gencode(f);

	int label1 = getLabel();
	int label2 = getLabel();

	emit(f, "isub");
	emitJump(f, "ifne", label1);
	emit(f, "ldc", 0);
	emitJump(f, "goto", label2);
	emitLabel(f, label1);
	emit(f, "ldc", 1);
	emitLabel(f, label2);
}



