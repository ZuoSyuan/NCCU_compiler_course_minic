#ifndef EXPR_H
#define EXPR_H

#include "minic.h"



class Expr { // abstract base class for expressions
	protected:
		Expr_Type type;
	public:
		Expr(Expr_Type type_) : type(type_) {}
		Expr_Type getType() const;
		void setType(Expr_Type type_);
		virtual void gencode(FILE *f) const = 0; // pure virtual method
};



class IntConstExpr : public Expr {
	protected:
		int value;
	public:
		IntConstExpr(int value_) : Expr(INT_TYPE), value(value_) {}
		void gencode(FILE* f) const;
};

class BoolConstExpr : public Expr {
	protected:
		bool value;
	public:
		BoolConstExpr(bool value_) : Expr(BOOL_TYPE), value(value_) {}
		void gencode(FILE* f) const;
};

///
class FloatConstExpr : public Expr {
	protected:
		float value;
	public:
		FloatConstExpr(float value_) : Expr(FLOAT_TYPE), value(value_) {}
		void gencode(FILE* f) const;
};

class CallExpr : public Expr {
	protected:
		string ident;
		vector<Expr*>* args;
	public:
		CallExpr(string ident_, vector<Expr*>* args);
		void gencode(FILE* f) const;
};

class LocalDeclExpr : public Expr {
	protected:
		string ident;
		int offset;
	public:
		LocalDeclExpr(string ident_, Expr_Type type_);
		void gencode(FILE* f) const;
};

class IdentExpr : public Expr {
	protected:
		string ident;
		int index;
		int isGlobal;
	public:
		IdentExpr(string ident_);
		void gencode(FILE* f) const;
};


///ArrayDeclExpr
class ArrayDeclExpr : public Expr {
	protected:
		string ident;
		int offset;
	public:
		ArrayDeclExpr(string ident_, Expr_Type type_);
		void gencode(FILE* f) const;
};

///NewArrayExpr
class NewArrayExpr : public Expr {
	protected:
		Expr* indexexpr;
	public:
		NewArrayExpr(Expr_Type type_, Expr* indexexpr_);
		void gencode(FILE* f) const;
};

///GetArraySizeExpr
class GetArraySizeExpr : public Expr {
	protected:
		string ident;
		int index;
		int isGlobal;
		Expr_Type oritype;
	public:
		GetArraySizeExpr(string ident_);
		void gencode(FILE* f) const;
};

///IdentArrayExpr
class IdentArrayExpr : public Expr {
	protected:
		string ident;
		Expr* indexexpr;
		int index;
		int isGlobal;
		Expr_Type oritype;
	public:
		IdentArrayExpr(string ident_, Expr* indexexpr_);
		void gencode(FILE* f) const;
};

///LeftValueExpr
class LeftValueExpr : public Expr {
	protected:
	public:
		string ident;
		Expr* indexexpr;
		Expr_Type oritype;
		bool isGlobal;
		int index;
		LeftValueExpr(string ident_, Expr* indexexpr_);
		void gencode(FILE* f) const;
};

class VarAssignmentExpr : public Expr {
	protected:
		Expr* left;
		Expr* right;
	public:
		VarAssignmentExpr(Expr* left_, Expr* right_);
		void gencode(FILE* f) const;
};


class UnaryExpr : public Expr {
	protected:
		Expr* right;
	public:
		UnaryExpr(Expr* right_) : Expr(GARBAGE_TYPE), right(right_) {}
		virtual void gencode(FILE *f) const = 0; // pure virtual method
};

class NegateUnaryExpr : public UnaryExpr {
	public:
		NegateUnaryExpr(Expr* right_);
		void gencode(FILE* f) const;
};

class AdditionUnaryExpr : public UnaryExpr {
	public:
		AdditionUnaryExpr(Expr* right_);
		void gencode(FILE* f) const;
};

class SubtractUnaryExpr : public UnaryExpr {
	public:
		SubtractUnaryExpr(Expr* right_);
		void gencode(FILE* f) const;
};


	

class BinaryExpr : public Expr {
	protected:
		Expr* left;
		Expr* right;

	public:
		BinaryExpr(Expr* left_, Expr* right_) : Expr(GARBAGE_TYPE), left(left_), right(right_) {}
		virtual void gencode(FILE *f) const = 0; // pure virtual methods
};

class NumberBinaryExpr : public BinaryExpr {
	public:
		bool left_i2f;
		bool right_i2f;
		NumberBinaryExpr(Expr* left_, Expr* right_);
		virtual void gencode(FILE *f) const = 0; // pure virtual method
};

class AdditionBinaryExpr : public NumberBinaryExpr {
	public:
		AdditionBinaryExpr(Expr* left_, Expr* right_) : NumberBinaryExpr(left_, right_) {}
		void gencode(FILE *f) const;
};

class SubtractionBinaryExpr : public NumberBinaryExpr {
	public:
		SubtractionBinaryExpr(Expr* left_, Expr* right_) : NumberBinaryExpr(left_, right_) {}
		void gencode(FILE *f) const;
};

class MultiplicationBinaryExpr : public NumberBinaryExpr {
	public:
		MultiplicationBinaryExpr(Expr* left_, Expr* right_) : NumberBinaryExpr(left_, right_) {}
		void gencode(FILE *f) const;
};

class DivisionBinaryExpr : public NumberBinaryExpr {
	public:
		DivisionBinaryExpr(Expr* left_, Expr* right_) : NumberBinaryExpr(left_, right_) {}
		void gencode(FILE *f) const;
};

class ModulusBinaryExpr : public NumberBinaryExpr {
	public:
		ModulusBinaryExpr(Expr* left_, Expr* right_) : NumberBinaryExpr(left_, right_) {}
		void gencode(FILE *f) const;
};






class LogicBinaryExpr : public BinaryExpr {
	public:
		LogicBinaryExpr(Expr* left_, Expr* right_);
		virtual void gencode(FILE *f) const = 0; // pure virtual method
};

class OrBinaryExpr : public LogicBinaryExpr {
	public:
		OrBinaryExpr(Expr* left_, Expr* right_) : LogicBinaryExpr(left_, right_) {}
		void gencode(FILE* f) const;
};

class AndBinaryExpr : public LogicBinaryExpr {
	public:
		AndBinaryExpr(Expr* left_, Expr* right_) : LogicBinaryExpr(left_, right_) {}
		void gencode(FILE* f) const;
};

class CompareBinaryExpr : public BinaryExpr {
	public:
		CompareBinaryExpr(Expr* left_, Expr* right_);
		virtual void gencode(FILE *f) const = 0; // pure virtual method
};

class LECompareBinaryExpr : public CompareBinaryExpr {
	public:
		LECompareBinaryExpr(Expr* left_, Expr* right_);
		void gencode(FILE* f) const;
};

class LTCompareBinaryExpr : public CompareBinaryExpr {
	public:
		LTCompareBinaryExpr(Expr* left_, Expr* right_);
		void gencode(FILE* f) const;
};

class GECompareBinaryExpr : public CompareBinaryExpr {
	public:
		GECompareBinaryExpr(Expr* left_, Expr* right_);
		void gencode(FILE* f) const;
};

class GTCompareBinaryExpr : public CompareBinaryExpr {
	public:
		GTCompareBinaryExpr(Expr* left_, Expr* right_);
		void gencode(FILE* f) const;
};

class EQCompareBinaryExpr : public CompareBinaryExpr {
	public:
		EQCompareBinaryExpr(Expr* left_, Expr* right_);
		void gencode(FILE* f) const;
};

class NECompareBinaryExpr : public CompareBinaryExpr {
	public:
		NECompareBinaryExpr(Expr* left_, Expr* right_);
		void gencode(FILE* f) const;
};







#endif /* EXPR_H */

