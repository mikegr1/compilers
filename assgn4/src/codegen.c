#include <tree.h>
#include <strtab.h>
#include <codegen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* file;

extern char *nodeNames[];
extern DataItem *symtable[];
extern char * typeNames[];
extern char * nodeKindNames[];

struct actRecord *head = NULL;
struct actRecord *current = NULL;
// Used Labels 
int uLabels = 0;
int uReg = 0;

// Next Register
int nReg()
{
   uReg++;
   return uReg;
}
// resets register
void rReg()
{
   uReg = 0;
}
// new Labels
int nLabel()
{
   uLabels++;
   return uLabels;
}

// Goes through the symbol table and adds any global variables to the data section. 
void global_variables(){
  int varN = 0;	
  int index = 0;
  while (current_scope->strTable[index] != 'l'){
    if (strcmp(nodeNames[nodeKind], "var") == 0){
       if (varN == 0){
          fprintf(file, "\t%s\n", ".data");
       }
       varN++;
       int length = strlen(symtable[index]->id) + 1;
       char global_lab[length];
       strncat(global_lab, symtable[index]->id, strlen(symtable[index]->id));
       strncat(global_lab, ":", 1);
       fprintf(file, "%-20.20s.word    0\n", global_lab);
    }
    index++;
  }
}
// Looks for function declerations to create a linked list of the activation records. 
void findFunCalls(tree *node, int nestLevel)
{
   int i;
   char *nodeType = nodeNames[node->nodeKind]; 

   if (strcmp(nodeType, "funcDecl") == 0)
   {
     char * funcName = getSecondChild(node)->name;
     int symIndex = ST_lookup(funcName, symtable);
     addActList(node, funcName, symIndex);
   }
   for (i = 0; i < node->numChildren; i++)  
  { findFunCalls(getChild(node, i), nestLevel + 1); }
}

void gen(tree *node, int nestLevel)
{  
  int i;
  char *nodeType = nodeNames[node->nodeKind]; 

  if (strcmp(nodeType, "funcDecl") == 0)
  {
     emitFunc(node, getSecondChild(node)->name);
  }

  for (i = 0; i < node->numChildren; i++)  
  {
    gen(getChild(node, i), nestLevel + 1);
  }
}

void genCode(tree *node)
{
     if(!node)
     { return; }
     emitPre();
     global_variables();
     findFunCalls(node, 1);
     if (head)
     {
        fprintf(file, "\t.text\n");
     }
     gen(node, 1);
     emitPost();
}

int calSpace(int numVars)
{
   int locals = numVars * 4;
   return locals + 24;
   
}

// Offset is returned from position of AR. 
int offset(int varPosition, int calculatedSpace)
{
   int offset;
   if (varPosition == 0)
   {
     offset = calculatedSpace - 4;
   }
   else
   {
      offset = calculatedSpace - 4 - (varPosition * 4);
   }
   return offset;
}

void emitPre()
{
   file = fopen("out.asm", "w"); 
   fprintf(file, "\tGenerated code begining.\n");
}

void emitPost()
{
   fprintf(file, "\tEnd of generated code.\n");
   fclose(file);
}

// fundecl is passed in as a node.
void emitFunc(tree *node, char * funcName)
{
  actRecord * AR = findAR(funcName);
  if (strcmp(funcName, "main") == 0)
  {
     fprintf(file, "\t.globl   main\n");
  }

  // Emits a label.
  fprintf(file, "%s:\n", AR->funcName);
  fprintf(file, "\t# Function entry %s\n", AR->funcName);
  
  // Creates memSpace for activation actRecord. 
  fprintf(file, "\tsubi $sp, $sp, %d\n\n", AR->memSpace);
   
  // Saves the registers for the local variables.
  fprintf(file, "\t# save registers\n");
  fprintf(file, "\tsw $fp, %d($sp)\n", AR->memSpace);
  int i;
  for (i = 0; i < AR->numLocals; i++)
  {
     int varOffset = offset(i + 1, AR->memSpace);
     fprintf(file, "\tsw $t%d, %d($sp)\n", AR->locals[i]->numRegister, varOffset);
  }
  
  // Saves register for the return value.
  int retOffset = offset(AR->numLocals + 1, AR->memSpace);
  fprintf(file, "\t# save register for return value\n");
  fprintf(file, "\tsw $ra, %d($sp)\n", retOffset);

 // Sets the frame pointer. 
  fprintf(file, "\taddi, $fp, $sp, %d\n\n", AR->memSpace);

  fprintf(file, "\t# function body\n");
  tree *funBody = getChildByKind(node, "funBody");
  
  tree *stateList = getChildByKind(funBody, "statementList");
  if (stateList)
  {
     emitStmts(AR, stateList, 1);
  }

 fprintf(file, "\t# function exit for %s\n", funcName);
 if (strcmp(funcName, "main") == 0)
 {
   fprintf(file, "L0001:\n");
 }
// Restore the registers 
 fprintf(file, "\t#Restoring registers\n");
 fprintf(file, "\tlw $fp, %d($sp)\n", AR->memSpace);
 int j;
  for (j = 0; j < AR->numLocals; j++)
  {
     int varOffset = offset(j + 1, AR->memSpace);
     fprintf(file, "\tlw $t%d, %d($sp)\n", AR->locals[j]->numRegister, varOffset);
  }
// Restore the SP 
 fprintf(file, "\taddi $sp, $sp, %d\n", AR->memSpace); 

if (strcmp(funcName, "main") == 0)
 {
   fprintf(file,"\tli $v0, 17\n");
   fprintf(file, "\tsyscall\n");   
 }
 else
 {
 fprintf(file, "\tjr $ra\n");
 }
  
 fprintf(file, "\t# end %s()\n", funcName);
 // reset registers 
 rReg();
} 

void emitStmts(actRecord * AR, tree *node,  int nestLevel)
{
   if (isNodeName(node, "assignStmt") == 0)
   {
       tree * firstChild = getFirstChild(node);
      if (isNodeName(firstChild, "identifier") == 0)
      {
         tree * secondChild = getSecondChild(node);
        // Assigment statements
         if (isNodeName(secondChild, "intConst") == 0 || isNodeName(secondChild, "identifier") == 0 )
         {
            emitAssign(AR, firstChild, secondChild);
         }
        // Expressions and factors 
         if (isNodeName(secondChild, "+") == 0 || isNodeName(secondChild, "-") == 0 || isNodeName(secondChild, "*") == 0 || isNodeName(secondChild, "/") == 0)
         {
            fprintf(file, "\n\t# Expression\n");
            emitExpr(AR, secondChild);
         }        
     }
     // function calls
     if (strcmp(nodeNames[firstChild->nodeKind], "funcCallExpr") == 0)
     {
        emitFuncCall(AR, firstChild);
     } 
   }

    // conditionals
     
     if (strcmp(nodeNames[node->nodeKind], "condStmt") == 0)
     { 
         tree * firstChild = getFirstChild(node);
         
         //if Else
         emitExpr(AR, firstChild);
     }
     // while only
     if (strcmp(nodeNames[node->nodeKind], "loopStmt") == 0)
     {
        emitLoops();
     }
     
  int i;
  for (i = 0; i < node->numChildren; i++)  
  { 
      emitStmts(AR, getChild(node, i), nestLevel + 1);
  }
}

void emitAssign(actRecord * AR, tree * identifierNode, tree * valueNode)
{
   // local variable. 
   DataItem * identifier = lookupARLocal(AR, identifierNode->name);
   int globalFlag = 0;
   if(!identifier)
   {
    // global variable. 
      int index = ST_lookup(identifierNode->name, symtable);
      identifier = symtable[index];
      identifier->numRegister = nReg();
      globalFlag = 1;
   }
   int value; 
   if (strcmp(nodeNames[valueNode->nodeKind], "identifier") == 0)
   {
      DataItem * valID = lookupARLocal(AR, valueNode->name);
      if(!valID)
      {
         int index = ST_lookup(valueNode->name, symtable);
         valID = symtable[index];

      }
      value = valID->scalarValue;
   }
   if (strcmp(nodeNames[valueNode->nodeKind], "intConst") == 0)
   {
      value = valueNode->val;
   }
   fprintf(file, "\n\t# Assignment for %s\n", identifier->key);
   if (globalFlag == 1)
   {
      fprintf(file, "\tla $t%d, %s\n", identifier->numRegister, identifier->key); 
   }
   else
   {
      fprintf(file, "\tli $t%d, %d\n", identifier->numRegister, value);
   }
   fprintf(file, "\tsw $t%d, %d($sp)\n", identifier->numRegister, offset(identifier->numRegister, AR->memSpace));
   
}
    
void emitFuncCall(actRecord * AR, tree * funcCallNode)
{  
   tree * funcIdNode = getFirstChild(funcCallNode);
   tree * argNode = getSecondChild(funcCallNode);
   int argValue;
   
   if(strcmp(nodeNames[argNode->nodeKind], "identifier")  == 0)
   {
       // looks for a local variable.
       DataItem * arg = lookupARLocal(AR, argNode->name);
       if (arg)
       {   
           argValue = arg->scalarValue;
       }  
       // looks for a global variable.
       else
       {
          int index = ST_lookup(argNode->name, symtable);
          if (index != -1)
          {
             argValue = symtable[index]->scalarValue;
          }
       }
   }
   if (strcmp(nodeNames[argNode->nodeKind], "intConst") == 0)
   {
      argValue = argNode->val; 
   }
   if (strcmp(funcIdNode->name, "output") == 0)
   {
      emitOutput(argValue);
   }
   else
   {
     fprintf(file, "\tli $a0, %d\n", argValue);
     fprintf(file, "\tjal %s\n", funcIdNode->name);
     fprintf(file, "\tmove $t0, $v0\n");
   }

}       

//  For an expression node assignment node, the AST contains two operands. Where there can be two operands such as an identifier, a constant, or expression node that undergo arithmetic operation. 
int emitExpr (actRecord * AR, tree * exprNode)
{
   int result, t1, t2;
 
   if (isNodeName(exprNode, "+") == 0 || isNodeName(exprNode, "-") == 0 || isNodeName(exprNode, "*") == 0 || isNodeName(exprNode, "/") == 0)
   {
      t1 = emitExpr(AR, getFirstChild(exprNode));
      t2 = emitExpr(AR, getSecondChild(exprNode));
    // Addition, subtraction, division, and multiplication.
       if (isNodeName(exprNode, "*") == 0 )
      {
         result = nReg();
         fprintf(file, "\tmul $t%d, $t%d, $t%d\n", result, t1, t2);
         fprintf(file, "\tsw $t%d, %d($sp)\n", result, offset(result, AR->memSpace));
      }
    
       if (isNodeName(exprNode, "/") == 0 )
      {
         result = nReg();
         fprintf(file, "\tdiv $t%d, $t%d, $t%d\n", result, t1, t2);
         fprintf(file, "\tsw $t%d, %d($sp)\n", result, offset(result, AR->memSpace));
      }
      if (isNodeName(exprNode, "+") == 0 )
      {
         result = nReg();
         fprintf(file, "\tadd $t%d, $t%d, $t%d\n", result, t1, t2);
         fprintf(file, "\tsw $t%d, %d($sp)\n", result, offset(result, AR->memSpace));
      }
      
      if (isNodeName(exprNode, "-") == 0 )
      {
         result = nReg();
         fprintf(file, "\tsub $t%d, $t%d, $t%d\n", result, t1, t2);
         fprintf(file, "\tsw $t%d, %d($sp)\n", result, offset(result, AR->memSpace));
      }
    }

     // Conditionals.
     if (isNodeName(exprNode, "<") == 0)
     {
       t1 = emitExpr(AR, getFirstChild(exprNode));
       t2 = emitExpr(AR, getSecondChild(exprNode));
       result = nReg();
       
       fprintf(file, "\n\t# Less than \n");
       fprintf(file, "\tslt $t%d, $t%d, $t%d\n", result, t1, t2); 
       fprintf(file, "\tbeq $t%d, 1, L%d\n", result, nlabel());
     }
     if (isNodeName(exprNode, ">") == 0)
     {
       t1 = emitExpr(AR, getFirstChild(exprNode));
       t2 = emitExpr(AR, getSecondChild(exprNode));
       result = nReg();
       
       fprintf(file, "\n\t# Greater than \n");
       fprintf(file, "\tslt $t%d, $t%d, $t%d\n", result, t2, t1); 
       fprintf(file, "\tbeq $t%d, 1, L%d\n", result, nlabel());
     }
    if (isNodeName(exprNode, "==") == 0)
     {
       t1 = emitExpr(AR, getFirstChild(exprNode));
       t2 = emitExpr(AR, getSecondChild(exprNode));
       result = nReg(); 
      
       
       fprintf(file, "\n\t# == \n");
       fprintf(file, "\tbeq $t%d, $t%d, L%d\n", t1, t2, nlabel()); 
     }
    if (isNodeName(exprNode, "identifier") == 0)
    {
       DataItem * identifier = lookupARLocal(AR, exprNode->name);
       if (identifier)
       {
          result = identifier->numRegister;
       }
       else
       {
         int index = ST_lookup(identifier->key, symtable);
         identifier = symtable[index];
         result = identifier->numRegister;
       }
    }
    if (isNodeName(exprNode, "intConst") == 0)
    {
       fprintf(file, "\tli $t%d, %d\n", result, exprNode->val);
    }
    // Expression.
    if (isNodeName(exprNode, "factor") == 0)
    {
       result = emitExpr(AR, getFirstChild(exprNode));
    }
    return result;  
}

void emitOutput(int x)
{
   fprintf(file, "\n\t# output\n");
   fprintf(file, "\tli $v0, 1\n");
   fprintf(file, "\tli $a0, %d\n", x);
   fprintf(file, "\tsyscall\n\n");
}


// A linked list of activation records is created with the use of the symbol table and Abstract Syntax tree as an intermediate format to gen code. 
void addActList(tree *node, char * funcName, int symIndex)
{
   
   // Creates an activation record. 
   actRecord *newRecord;
   newRecord = (actRecord *) malloc(sizeof(actRecord));
   newRecord->funcName = funcName;
   newRecord->numLocals = 0;

   tree * argument = getChildByKind(node, "formalDecl");
   if (argument)
   {
      int ind = ST_lookup(getSecondChild(argument)->name, symtable[symIndex]->localtable);
      newRecord->param = symtable[symIndex]->localtable[ind];
   }

   tree * funcBody = getChildByKind(node, "funBody");
   
   // Looks for local variables.
   tree * localDeclList = getChildByKind(funcBody, "localDeclList");
   if (localDeclList)
   {
      newRecord->numLocals = walkSubTree(localDeclList, "varDecl", 1); 

      newRecord->locals = malloc(sizeof(DataItem) * newRecord->numLocals);
      int j;
      int k = 0;
      for (j = 0; j < symtable[symIndex]->localSymbolCount; j++)
      {
         // Symbol Table order is matched with the function body order. 
         if (strcmp(nodeKindNames[symtable[symIndex]->localtable[j]->kind], "var") == 0)
         {
            newRecord->locals[k] = symtable[symIndex]->localtable[j];
            
           // Stores registers. 
            symtable[symIndex]->localtable[j]->numRegister = k; 
            k++;
            nReg();
         }
      }
   }
   // Memory Space is calcuated. 
   newRecord->memSpace = calSpace(newRecord->numLocals);
  
   tree * statementList = getChildByKind(funcBody, "statementList");
   if (statementList)
   {
      tree * returnNode = getNestedChildByKind(statementList, "returnStmt", 1);
      if(returnNode)   
      { 
         tree * intConst = getChildByKind(returnNode, "intConst"); 
	 if (intConst)
	 {
            newRecord->returnValue = getFirstChild(returnNode)->val;
	 }
	 tree * returnVar = getChildByKind(returnNode, "identifier");
	 if (returnVar)
	 {
            int ind = ST_lookup(returnVar->name, symtable);
	    if (ind != -1)
            {
               newRecord->returnValue = symtable[ind]->scalarValue;
 	    }
	    else
	    {
	       ind = ST_lookup(returnVar->name, symtable[symIndex]->localtable);
               newRecord->returnValue = symtable[symIndex]->localtable[ind]->scalarValue;
	    } 
        }
     }
  }
  
  if (head)
  {
     current->next = newRecord;
     current = current->next; 
  }
  else
  {
     head = newRecord;
     current = head;
  }
}

struct actRecord * findAR(char * funcName)
{
   struct actRecord * temp;
   temp = (actRecord *) malloc(sizeof(actRecord));
   temp = head;
   while (temp)
   {
      if (strcmp(funcName, temp->funcName) == 0)
      { 
        return temp; 
        break; 
      }
      else
      { temp = temp->next; }
   }
}

// Looks up local variable in the Activation Record's local array. 
DataItem * lookupARLocal(actRecord * AR, char * name)
{
   DataItem * isFound;
   int i;
   int index = -1;

   for (i = 0; i < AR->numLocals; i++)
   {

      if (strcmp(AR->locals[i]->key, name) == 0)
      {
         isFound = AR->locals[i];
         index = i;
         break;
      }
   }
   if (index == -1)
   { isFound = NULL; }
   return isFound;
}

// Prints most recent Activation record. 
void printARs()
{
   actRecord * temp = head;
   printf("\nActive Records\n");
   while (temp)
   {
      printf("Activation record for: %s \n", temp->funcName);
      printf("The return value: %d \n", temp->returnValue);
      if (temp->param)
      {
      	  printf("Parameter name: %s \n", temp->param->key);
      }
      if (temp->locals)
      {
         int i;
         for (i = 0; i < temp->numLocals; i++)
         {
            printf("Local Variable %d: %s \n", i, temp->locals[i]->key);
         }
      }
      printf("Memory Space: %d \n", temp->memSpace);
      printf("\n");
      temp = temp->next;
   }
}
