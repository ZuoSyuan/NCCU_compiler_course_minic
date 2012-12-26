#include "symbol.h"



SymbolTable::SymbolTable(SymbolTable* parent_) : parent(parent_), currentOffset(0), maxLocalCount(0), depth(0) {
	//If there is a parent then grab its offsets
	if(parent != NULL && parent->getParent() != NULL)
	{
		currentOffset = parent->getOffset();
		maxLocalCount = parent->getMaxLocalCount();
		depth = parent->getDepth() + 1;
	}
}

// Search the symbol table recursively for the function
FunctionEntry* SymbolTable::getFunctionEntry(string ident) {

	map<string,SymbolEntry*>::iterator iter = entries.find(ident);
	if( iter != entries.end() ) {
		//C++ magic to check if instance of
		SymbolEntry* newSymbol = iter->second;
		FunctionEntry* newFunction = dynamic_cast<FunctionEntry*>(newSymbol);
		return newFunction;
	}
	else
	{
		if(parent != NULL) return parent->getFunctionEntry(ident);
		else return NULL;
	}
}

// Search the symbol table recursively for the variable
VariableEntry* SymbolTable::getVariableEntry(string ident) {

	map<string,SymbolEntry*>::iterator iter = entries.find(ident);
	if( iter != entries.end() ) {
		//C++ magic to check if instance of
		SymbolEntry* newSymbol = iter->second;
		VariableEntry* newVariable = dynamic_cast<VariableEntry*>(newSymbol);
		return newVariable;
	}
	else
	{
		if(parent != NULL) return parent->getVariableEntry(ident);
		else return NULL;
	}
}

bool SymbolTable::insertFunctionEntry(string ident_, vector<Expr_Type>* args_, Expr_Type returnType_) {

	//set up our variables I.E. C++ MAGIC
   	typedef pair <string, SymbolEntry*> Entry_Pair;
	pair< map<string,SymbolEntry*>::iterator, bool > pair_return;

	//Construct a new Function Entry
	FunctionEntry* newEntry = new FunctionEntry(ident_, args_, returnType_);

	//Attempt to insert and get the crazy return value
	pair_return = entries.insert(Entry_Pair(ident_, newEntry));

	//return the insert status bool
	return pair_return.second;

}

bool SymbolTable::insertVariableEntry(string ident_, Expr_Type type_) {

	//set up our variables I.E. C++ MAGIC
	typedef pair <string, SymbolEntry*> Entry_Pair;
	pair< map<string,SymbolEntry*>::iterator, bool > pair_return;
	bool global_;

	//Detect if this variable is global by knowing if their is a parent symbol table
	int offset = 0;
	if(parent == NULL) {
		global_ = true;
	}
	else {
		global_ = false;
		offset = getNewOffset();
	}
	
	//Construct a new Function Entry
	VariableEntry* newEntry = new VariableEntry(ident_, type_, global_, offset);

	//Attempt to insert and get the crazy return value
	pair_return = entries.insert(Entry_Pair(ident_, newEntry));

	//return the insert status bool
	return pair_return.second;

}

int SymbolTable::getOffset() { 
	return currentOffset;
}

int SymbolTable::getNewOffset() {
	int oldOffset = currentOffset;
	currentOffset++;
	setMaxLocalCount(currentOffset);
	return oldOffset;
}

SymbolTable* SymbolTable::getParent() {
	return parent;
}

int SymbolTable::getMaxLocalCount() { 
	return maxLocalCount;
}

void SymbolTable::setMaxLocalCount(int newLocalCount) {
	if(maxLocalCount < newLocalCount)
		maxLocalCount = newLocalCount;
}

int SymbolTable::getDepth() {
	return depth;
}

Expr_Type SymbolEntry::getType() {
	return type;
}

FunctionEntry::FunctionEntry(string ident_, vector<Expr_Type>* args_, Expr_Type returnType_) : SymbolEntry(ident_, returnType_), args(args_) {

}


vector<Expr_Type>* FunctionEntry::getArgs() {
	return args;
}

VariableEntry::VariableEntry(string ident_, Expr_Type type_, bool global_, int index_) : SymbolEntry(ident_, type_), global(global_), index(index_) {


}

int VariableEntry::getIndex() {
	return index;
}

bool VariableEntry::isGlobal() {
	return global;
}

