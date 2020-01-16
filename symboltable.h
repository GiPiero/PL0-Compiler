#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#define NAME_BUFF_LEN 11

typedef enum {CONSTANT, VARIABLE, PROCEDURE} s_type;

typedef struct Symbol {
	struct Symbol *prev_node;
	char name[NAME_BUFF_LEN];
	int val;
	int lvl;
	int adr;
	s_type type;
} Symbol;

typedef struct SymbolTable {
	Symbol **h_list;
	int max_hash;
	int size;
	// int top_level?
} SymbolTable;

Symbol *add_symbol(SymbolTable *st, s_type type, char *name, int val, int lvl, int adr);
Symbol *get_symbol(SymbolTable *st, char *name);
SymbolTable *destroy_st(SymbolTable *st);
SymbolTable *new_st(int max_hash);
void remove_level(SymbolTable *st, int level);

#endif
