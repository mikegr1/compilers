#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "strtab.h"
#include <iostream>

/* Inserts a symbol into the  symbol table tree. Please note that this function is used to instead into the tree of symbol tables and NOT the AST. Start at the returned hash and probe until we find an empty slot or the id.  */
int ST_insert(char *id, int data_type, int symbol_type, int* scope){
    if(ST_lookup(id) != NULL){
        return -1;
    }
    symEntry * new_insertion;
    new_insertion = malloc(sizeof(symEntry));
    new_insertion->id = id;
    new_insertion->data_type = data_type;
    new_insertion->symbol_type = symbol_type;
    new_insertion->scope = scope;
    if(new_insertion->symbol_type == FUNCTION){
        param* temp = working_list_head;
        temp = malloc(sizeof(param));
        int count = 0;
        while(temp != NULL){
            temp = temp -> next;
            count++;
        }
        new_insertion->size = count;
         free(temp);
    }
    else if(new_insertion->symbol_type == SCALAR){
        new_insertion->size = 1;
    }

    int x = 0;
    while(current_scope->strTable[x] != NULL){
        x++;
    }
    current_scope -> strTable[x] = new_insertion;
    free(new_insertion);
    return new_insertion->size;
}

int ST_insert_array(char *id, int data_type, int symbol_type, int* scope, int size){
    if(ST_lookup(id) != NULL){
        return -1;
    }
    symEntry * new_insertion;
    new_insertion = malloc(sizeof(symEntry));
    new_insertion->id = id;
    new_insertion->data_type = data_type;
    new_insertion->symbol_type = symbol_type;
    new_insertion->scope = scope;
    new_insertion->size = size;
    int x = 0;
    while(current_scope->strTable[x] != NULL){
        x++;
    }
    current_scope -> strTable[x] = new_insertion;
    free(new_insertion);
    return new_insertion->size;
}



/* The function for looking up if a symbol exists in the current_scope. Always start looking for the symbol from the node that is being pointed to by the current_scope variable*/
symEntry* ST_lookup(char *id){
        for(int b = 0; b < sizeof(current_scope->strTable)/sizeof(current_scope->strTable[0]); b++){
            if(strcmp(current_scope->strTable[b]->id, id) == 0){
                return (current_scope->strTable);
            }
            else{
                return NULL;
        }     
    }
}

/* Creates a param* whenever formalDecl in the parser.y file declares a formal parameter. Please note that we are maining a separate linklist to keep track of all the formal declarations because until the function body is processed, we will not know the number of parameters in advance. Link list provides a way for the formalDecl to declare as many parameters as needed.*/

void add_param(int data_type, int symbol_type){
    param * p;
    p = malloc(sizeof(param));
    param * t;
    t = malloc(sizeof(param));
    p ->data_type = data_type;
    p ->symbol_type = symbol_type;
    t = working_list_head -> next;
    if(working_list_head -> next == NULL){
        working_list_head -> next = p;
        p -> next = working_list_end;
    }
    else{
        while(t != NULL){
            t = t -> next;
        }
        if(t -> next == NULL){
            t -> next = p;
            p -> next = working_list_end;
        }
    }
    free(t);
    free(p);
}

/*connect_params is called after the funBody is processed in parser.y. At this point, the parser has already seen all the formal parameter declaration and has built the entire list of parameters to the function. This list is pointed to by the working_list_head pointer. current_scope->parent->strTable[index]->params should point to the header of that parameter list. */
void connect_params(int i, int num_params){
    current_scope->parent->strTable[i]->params = working_list_head;
    int x = 0;
    while(x < num_params){
        if(working_list_head -> next != working_list_end){
        current_scope->parent->strTable[i]->params-> next = working_list_head -> next;
        working_list_head = working_list_head -> next;
        x++;
        }
        else if(working_list_head -> next == working_list_end){
            x = num_params;
        }
    }

}

//how to implement scope: an enumeration, integer, or string/character
//this function should set the current_scope pointer to point at the next child of the current table_node
//this means that the scope in this function is going to be attributed in this function to the current table nodes symbolTable entries
//this means that the global table already has to be created for this function to work 
void new_scope(){
    current_scope -> numChildren += 1;
    table_node * new_table;
    new_table = malloc(sizeof(table_node));
    new_table -> parent = current_scope;
    current_scope -> next = new_table;
    current_scope = current_scope -> next;
    free(new_table);
}

// Moves towards the root of the strTable table tree.
void up_scope(){
    current_scope = current_scope -> parent;
}