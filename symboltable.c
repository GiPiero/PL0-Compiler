#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "symboltable.h"

// Clean-up stuff
Symbol *destroy_symbol(Symbol *sym)
{
	if(sym)	free(sym);
	return NULL;
}

SymbolTable *destroy_st(SymbolTable *st)
{
	Symbol *tmp;
	int i;

	if(!st) return NULL;

	if(st->h_list)
	{
		for(i = 0; i < st->max_hash; i++)
		{
			while(tmp = st->h_list[i])
			{
				st->h_list[i] = tmp->prev_node;
				destroy_symbol(tmp);
			}
		}
		free(st->h_list);
	}
	free(st);
	
	return NULL;
}

// Create new SymbolTable with an h_list capacity of max_hash
SymbolTable *new_st(int max_hash)
{
	SymbolTable *st = NULL;

	if( !(st = calloc(1, sizeof(SymbolTable))) ) 
		return NULL;

	st->max_hash = (max_hash < 1) ? 1 : max_hash;

	if( !(st->h_list = (Symbol **) calloc(st->max_hash, sizeof(Symbol *))) ) 
		return destroy_st(st);

	return st;
}

// Hash strings with djb2 algorithm
unsigned long hash(const char *str) 
{
	int c;
	unsigned long hash = 5381;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;

	return hash;
}

// Get the highest leveled symbol called name 
Symbol *get_symbol(SymbolTable *st, char *name)
{
	Symbol *tmp;
	int h;

	if( !st || !name )
		return NULL;

	// Locate the list name hashes to
	h = hash(name) % st->max_hash;

	if(!(tmp = st->h_list[h])) 
		return NULL;

	// Probe the list for the first symbol called "name"
	do
		if( !strcmp(tmp->name, name) ) return tmp;
	while( tmp = tmp->prev_node );

	return NULL;
}

// TODO: int expand_hash_arr(SymbolTable *st); ?

Symbol *add_symbol(SymbolTable *st, s_type type, char *name, int val, int lvl, int adr)
{
	int h;

	Symbol *s;
	
	if( !st || !name ) 
		return NULL;

	// Check for a symbol "name" with greater or equal level
	if( (s = get_symbol(st, name)) && s->lvl >= lvl ) 
		return NULL;

	// Create new symbol
	if( !(s = (Symbol *) calloc(1, sizeof(Symbol))) ) 
		return NULL;

	if( !strcpy(s->name, name) ) 
		return destroy_symbol(s);

	s->type = type;
	s->val = val;
	s->lvl = lvl;
	s->adr = adr;

	// Place new symbol at the head of its hash list
	h = hash(name) % st->max_hash;

	if( st->h_list[h] ) 
		if( st->h_list[h]->lvl <= s->lvl )
			s->prev_node = st->h_list[h];
		else return destroy_symbol(s);

	st->size++;
	
	return (st->h_list[h] = s);
}

// Remove all symbols with greater or equal level
void remove_level(SymbolTable *st, int level)
{
	Symbol *tmp;
	int i;

	for( i = 0; i < st->max_hash; i++ ) 
		while( tmp = st->h_list[i] )
			if(tmp->lvl >= level)
			{
				st->h_list[i] = tmp->prev_node;
				destroy_symbol(tmp);
			} 
			else break;
}

// void print_h_list(SymbolTable *st, FILE *out)
// {
// 	Symbol *tmp;
// 	int i;

// 	if(!st || !st->h_list) return;

// 	fprintf(out, "\n Hash     Symbols");
// 	for(i = 0; i < st->max_hash; i++)
// 	{
// 		fprintf(out, "\n+----+\n|%4d|", i);

// 		for(tmp = st->h_list[i]; tmp; tmp = tmp->prev_node)
// 			fprintf(out, " -> (%d, %s)", tmp->lvl, tmp->name);
// 	}
// 	fprintf(out, "\n+----+\n");
// }