#include<tree.h>
#include<strtab.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


tree *maketree(int kind){

	tree* createdTree = (tree *)malloc(sizeof(tree));
	createdTree -> parent = NULL;
	createdTree ->  nodeKind = kind;
	createdTree -> numChildren = 0;
	return createdTree;
}

tree *maketreeWithVal(int kind, int val){

	tree* createdTreeWithVal = (tree *)malloc(sizeof(tree));
	createdTreeWithVal -> val = val;
	createdTreeWithVal -> parent = NULL;
	createdTreeWithVal -> numChildren = 0;
	createdTreeWithVal -> nodeKind = kind;
	return createdTreeWithVal;

}

void addChild(tree *parent, tree *child){
	if(parent -> numChildren >= MAXCHILDREN){
		return;
	}
	nextChildInLine(parent) = child;
	parent -> numChildren +=1;
	child -> parent = parent;	
	return;

}

void printAst(tree *root, int nestLevel){
	
	if(nestLevel == 1){
		for(int c = 0; c < root->numChildren; c++){
			for(int q = c; q < nestLevel; q++){
				printf( "%d, %d", root->nodeKind, root-> val);
				printAst(childGetter(root, c), nestLevel++);			
			}
		}
	}
return;
}