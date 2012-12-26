#ifndef SYMBOL_H
#define SYMBOL_H

#include "minic.h"
using namespace std;

class SymbolEntry;
class FunctionEntry;
class VariableEntry;

class SymbolTable {

	protected:
		map<string, SymbolEntry*, less<string> > entries;
		SymbolTable* parent;
		int currentOffset;
		int maxLocalCount;
		int depth;
	public:
		SymbolTable(SymbolTable* parent_);
		FunctionEntry* getFunctionEntry(string ident);
		VariableEntry* getVariableEntry(string ident);
		bool insertFunctionEntry(string ident_, vector<Expr_Type>* args_, Expr_Type returnType_);
		bool insertVariableEntry(string ident_, Expr_Type type_);
		int getOffset();
		int getNewOffset();
		SymbolTable* getParent();
		int getMaxLocalCount();
		void setMaxLocalCount(int maxLocalCount_);
		int getDepth();
};

class SymbolEntry {

	protected:
		string ident;
		Expr_Type type;

	public:
		SymbolEntry(string ident_, Expr_Type type_) : ident(ident_), type(type_) {}
		virtual ~SymbolEntry() { }
		Expr_Type getType();
};

class FunctionEntry : public SymbolEntry {
	protected:
		vector<Expr_Type>* args;
	public:
		FunctionEntry(string ident_, vector<Expr_Type>* args_, Expr_Type returnType_);
		vector<Expr_Type>* getArgs();
};

class VariableEntry : public SymbolEntry {
	protected:
		int index;
		bool global;
	public:
		VariableEntry(string ident_, Expr_Type type_, bool global_, int index_);
		int getIndex();
		bool isGlobal();
};


#endif /* SYMBOL_H */

