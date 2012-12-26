
Goals: 

1. 學習利用emit系列函數.
2. 完成在expr.cpp與stmt.cpp內的gencode methods.
3. 每個class都有setStacksize()與getStacksize()的method, 利用這兩個方法在expr.cpp與stmt.cpp內的各class constructors完成stacksize的計算, 關於如何計算請閱讀老師上課的投影片. 
	(建議該部份完成前, 可於FuncDeclStmt暫設定stacksize為100, 使gencode只要正確便能執行.)
4. 陣列相關之功能為額外加分.

----------------------------------------------------------------------------
emit functions and other help codes : (the code implementation is in parser.y)

extern int getLabel();
	取得標籤的流水號, 並將當前標籤號碼遞增.
extern void emit(FILE* f, const char* op);
	只印出一個字串, 用於pop, dup之類不帶參數的指令.
extern void emit(FILE* f, const char* op, int value);
	印出字串[SPACE]整數, 用於load與store系列之類帶一個整數參數的指令.
extern void emit(FILE* f, const char* op, float value);
	印出字串[SPACE]浮點數, 要讀入浮點數常數時使用.
extern void emit(FILE* f, const char* op, const char* ident);
	印出字串[SPACE]字串, 呼叫函式或全域變數時使用.
extern void emitJump(FILE* f, const char* op, int label);
	印出字串[SPACE]#_[LABEL], 因標籤實際使用是#_流水號, 當參數是標籤的指令時使用, 如ifeq, ifne, goto, ... . 
extern void emitLabel(FILE* f, int label);
	印出標籤.
extern void emitGlobal(FILE* f, string ident, Expr_Type type);
	印出全域變數. 

extern string filename;					
	存著處理檔案的名稱.
extern map<int, string> TypeTable;
	將Type轉成"int", "float"之類的字串.
extern map<int, string> TypeSignature;
	將Type轉成"I", "F", "V"之類JVM識別用的signature.

----------------------------------------------------------------------------

Required software:
You need to install the following software on your computer.

make
g++ (we'll use the c++ language in later programming assignments)
flex
bison
java (JRE, java Runtime Enviroment)

If you have Ubuntu OS installed, you can simply enter the following command to install
the above software.
$~ sudo apt-get install flex (or make, g++)

----------------------------------------------------------------------------

This assignment zip file includes the following files:

makefile
minic.h
lexer.l
parser.y
expr.cpp
expr.h
stmt.cpp
stmt.h
symbol.cpp
symbol.h

jasmin.jar
IOclass.class

and the following test files :

CountSqrt
	測試基本語法, 讀入一整數N, 輸出根號N, 並取整數位輸出. 
TestForloop
	測試for迴圈, 讀入一整數N, 輸出費伯納西數列從1到N(第1項為0).
TestFloat
	測試浮點數, 先輸出PI值, 讀入一浮點數R, 輸出R平方, 輸出半徑R之圓面積(2*PI*r*r).
	需注意二元運算式兩端一為整數與一為浮點數時, 需將整數轉為浮點數, 對應的指令是i2f.
TestArray
	測試陣列, 包含全域及本地的宣告, 初始化, 設值, 取值, 取陣列大小. 
	該程式讀入一整數N, 輸出從1到N的所有奇數.

And the excutable class of them.
You can excute example class and know the right results.

----------------------------------------------------------------------------

程式執行與驗證語法:



以要測試的檔案名稱為CountSqrt為例:

make
	編譯出minic執行檔
make run FILE=CountSqrt
	該指令一次執行下面三條指令
./minic CountSqrt >CountSqrt.j
java -jar jasmin.jar CountSqrt.j
java CountSqrt

若是minic compiler發現錯誤或是jasmin assembler發現錯誤, 執行單一條指令比較方便除錯.
當不確定codegen邏輯是否正確時, 直接修改j檔案的jasmin assembly, 再用jasmin.jar去組譯, 先找到正確的執行結果是一個建議的方法.

p.s. 語法中包含IO串流導向是linux script語法, windows下確定不行, mac下若有問題請再用moodle回報.

----------------------------------------------------------------------------
refrence resources:

http://jasmin.sourceforge.net/
jasmin官方網站, 有最詳盡的文件.

http://cs.au.dk/~mis/dOvs/jvmspec/ref-Java.html
jasmin指令規格, 比官方的容易看懂.

http://homepages.ius.edu/RWISMAN/C431/html/hw7.htm
如何計算stack size


