#ifndef CODEGEN_H
#define CODEGEN_H

typedef struct actRecord {
   char * funcName;
   char *returnAddress;
   int memSpace; 
   int returnValue;  
   int returnValueRegister;
   int numLocals; 
   struct actRecord *next;
   struct DataItem *param; 
   struct DataItem **locals;
}  actRecord; 

void emitFunc(tree *node, char * funcName);
void emitStmts(actRecord * AR, tree *node,  int nestLevel);
void emitAssign(actRecord * AR, tree * identifierNode, tree * valueNode);
void emitFuncCall(actRecord * AR, tree * funcCallNode);
int emitExpr(actRecord * AR, tree * exprNode);
void emitLoop(); 
void emitPre();
void emitPost();
void resetReg();
void findFunCalls(tree *node, int nestLevel);
void global_variables();
void gen(tree *node, int nestLevel);
void genCode(tree *node);
int calSpace(int numVars); 
int offset(int varPosition, int calculatedSpace);
void emitOutput(int x); 
void printARs();
struct actRecord * findAR(char * funcName);
void addActList(tree *node, char * funcName, int symIndex);
DataItem * lookupARLocal(actRecord * AR, char * name);
#endif
