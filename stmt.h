#ifndef STMT_H
#define STMT_H

#include "minic.h"
#include "expr.h"
#include "symbol.h"

class Stmt { // abstract base class for statements
	public:
		virtual void gencode(FILE *f) const = 0; // pure virtual method
};

class NullStmt : public Stmt {
	public:
		NullStmt() {}
		virtual void gencode(FILE *f) const { emit(f,"nop"); }
};

class ExprStmt : public Stmt {
	protected:
		Expr* expr;
	public:
		ExprStmt(Expr* expr_) : expr(expr_) {}
		virtual void gencode(FILE *f) const;
};

class IfStmt : public Stmt {
	protected:
		Expr* cond;
		Stmt* thenStmt;
		Stmt* elseStmt;
	public:
		IfStmt(Expr* cond_, Stmt* then_);
		IfStmt(Expr* cond_, Stmt* then_, Stmt* else_);
		virtual void gencode(FILE *f) const;
};

class WhileStmt : public Stmt {
	protected:
		Expr* cond;
		Stmt* stmt;
	public:
		WhileStmt(Expr* cond_, Stmt* stmt_);
		virtual void gencode(FILE *f) const;
};

class ReturnStmt : public Stmt {
	protected:
		Expr* returnExpr;
	public:
		ReturnStmt(Expr* returnExpr_);
		virtual void gencode(FILE *f) const;
};

class CompoundStmt : public Stmt {
	protected:
		vector<Stmt*>* stmts;
		vector<Expr*>* localDecls;
	public:
		CompoundStmt(vector<Expr*>* localDecls_, vector<Stmt*>* stmts_) : localDecls(localDecls_), stmts(stmts_) {}
		virtual void gencode(FILE *f) const;
};

class FuncDeclStmt : public Stmt {
	protected:
		Expr_Type returnType;
		string ident;
		vector<Expr_Type>* args;
		Stmt* body;
		int maxLocalVars;
		int maxLocalStacks;
	public:
		FuncDeclStmt(Expr_Type returnType_, string ident_, vector<Expr_Type>* args_, Stmt* body_);
		virtual void gencode(FILE *f) const;
};

class StaticVarDeclStmt : public Stmt {
	protected:
		Expr_Type type;
		string ident;
	public:
		StaticVarDeclStmt(Expr_Type type_, string ident_);
		virtual void gencode(FILE *f) const;
};

class ForStmt : public Stmt {
	protected:
		Expr* init;
		Expr* cond;
		Expr* variance;
		Stmt* body;
	public:
		ForStmt(Expr* init_, Expr* cond_, Expr* variance_, Stmt* body_);
		virtual void gencode(FILE *f) const;
};

class StaticArrDeclStmt : public Stmt {
	protected:
		Expr_Type type;
		string ident;
	public:
		StaticArrDeclStmt(Expr_Type type_, string ident_);
		virtual void gencode(FILE *f) const;
};

#endif /* STMT_H */
