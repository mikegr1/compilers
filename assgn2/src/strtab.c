#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "strtab.h"


/* Provided is a hash function that you may call to get an integer back. */
unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

int ST_insert(char *id, char *scope, int data_type, int symbol_type){
    // TODO: Concatenate the scope and id and use that to create the hash key
	int index;
	unsigned char* key = (unsigned char *)malloc(strlen(id) + strlen(scope));
	strcpy(key, id); //save id to symbol table
	strcat(key, scope); //add scope to the id string
    // TODO: Use ST_lookup to check if the id is already in the symbol table. If yes, ST_lookup will return an index that is not -1. if index != -1, that means the variable is already in the hashtable. Hence, no need to insert that variable again. However, if index == -1, then use linear probing to find an empty spot and insert there. Then return that index.
	int found_key = ST_lookup(id, scope);
	if(found_key > -1){
		index = found_key;
		free(key);
		return index;
	}
	//here we insert
	int insert_me = hash(key);
	while(strTable[insert_me].id != NULL){
		insert_me++;
		if(insert_me >= MAXIDS){
			break;
		}
	}
//allocate memory for scope and insert the scope into the symbol table
    strTable[insert_me].scope = malloc(strlen(scope));
    strcpy(strTable[insert_me].scope, scope);
    free(strTable[insert_me].scope);


    // allocate memory for id and insert data id into symbol table
    strTable[insert_me].id = malloc(strlen(id));
    strcpy(strTable[insert_me].id, id);
    free(strTable[insert_me].id);

	//add data_type and symbol type to the table as well
	strTable[insert_me].data_type = data_type;
	strTable[insert_me].symbol_type = symbol_type;
	
	//set index equal to the last symbol table entry so that st_insert returns index of latest added symbol
	index = insert_me;

	//empty the key so that new keys may be generated
	free(key);
	
    return index;
}

int ST_lookup(char *id, char *scope) {
    // TODO: Concatenate the scope and id and use that to create the hash key
	int index;
	unsigned char* key = (unsigned char *)malloc(strlen(id) + strlen(scope) + 2);
	strcpy(key, id); //save id to symbol table
	strcat(key, scope); //add scope to id string
	index = hash(key); //create index to search for
	int is_found = 0; //this will be used to exit if the symbol is found
	while(is_found == 0){
		if((index >= MAXIDS) || (strTable[index].id == NULL)){ //check if the index has an existing id or if the index contains no data
			index = -1;
			is_found = 1;
		}
		else if(strcmp(id, strTable[index].id) == 0){ //compare the id string and the id at the given index
			free(key);
			return index;
		}
		is_found = 1;
	 	
	}
    // TODO: Use the hash value to check if the index position has the "id". If not, keep looking for id until you find an empty spot. If you find "id", return that index. If you arrive at an empty spot, that means "id" is not there. Then return -1. 
	free(key);
    return index;
}
const char* types[] = {"int", "char", "void"};
const char* symTypeMod[] = {"SCALAR", "ARRAY", "FUNCTION"};

void output_entry(int i){
    printf("%d: %s ", i, types[strTable[i].data_type]);
    printf("%s:%s%s\n", strTable[i].scope, strTable[i].id, symTypeMod[strTable[i].symbol_type]);
}
