#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parsegen.h"
#include "lexicalAnalyzer.h"
#include "symboltable.h"

enum opcodes {	NON, LIT, OPR, LOD, STO, 
				CAL, INC, JMP, JPC, SIO 	};

enum iocodes {	WRT = 1, REA = 2, HLT = 3	};

enum mcodes {	RET, NEG, ADD, SUB, MUL, DIV, ODD, 
				MOD, EQL, NEQ, LSS, LEQ, GTR, GEQ 	};


// Syntax error handling
static char * const err[26] = {
"?", 														// 0
"Use = instead of :=.", 									// 1
"= must be followed by a number.", 							// 2
"Identifier must be followed by =.", 						// 3
"const, var, procedure must be followed by identifier.",	// 4
"Semicolon or comma missing.", 								// 5
"Incorrect symbol after procedure declaration.", 			// 6
"Statement expected.", 										// 7
"Incorrect symbol after statement part in block.", 			// 8
"Period expected.", 										// 9
"Semicolon between statements missing.", 					// 10
"Undeclared identifier.", 									// 11
"Assignment to constant or procedure is not allowed.", 		// 12
"Assignment operator expected.", 							// 13
"call must be followed by an identifier.", 					// 14
"Call of a constant or variable is meaningless.", 			// 15
"then expected.", 											// 16
"Semicolon or } expected.", 								// 17
"do expected.", 											// 18
"Incorrect symbol following statement.", 					// 19
"Relational operator expected.", 							// 20
"Expression must not contain a procedure identifier.", 		// 21
"Right parenthesis missing.", 								// 22
"The preceding factor cannot begin with this symbol.", 		// 23
"An expression cannot begin with this symbol.", 			// 24
"This number is too large." 								// 25
};

void error(const char * message)
{
	FILE * errorFile = fopen("ef", "w");
	printf("An error occurred while running parser: %s\n", message);
	fprintf(errorFile, "\nAn error occurred while running parser: %s\n", message);
	fprintf(outFile, "\nAn error occurred while running parser: %s\n", message);
	fclose(errorFile);
	exit(0);
}

// Code generation stuff
static int cx; // code index
static int code[MAX_CODE_LENGTH][3];
static int stack_size = 0;

void update_stack_size(int op, int lvl, int m)
{
	if(op == LIT || op == LOD || (op == SIO && m == REA))
		stack_size++;
	else if(op == STO || op == JPC || (op == SIO && m == WRT))
		stack_size--;
	else if(op == INC)
		stack_size += m;
	else if(op == OPR)
		if(m == RET) stack_size = 0;
		else if (m != NEG && m != ODD) stack_size--;
}

void emit(int op, int lvl, int m)
{
	if(cx >= MAX_CODE_LENGTH) 
		error("Exceeded MAX_CODE_LENGTH.");

	code[cx][0] = op;
	code[cx][1] = lvl;
	code[cx][2] = m;
	cx++;
	update_stack_size(op, lvl, m);
}

// File stuff
void print_assembly(FILE * out)
{
	int i;
	for(i = 0; i < cx; i++)
		fprintf(out, "%d %d %d\n", code[i][0], code[i][1], code[i][2]);
}

// Parsing stuff
SymbolTable *symbol_table = NULL;
static int tokval, level = -1;
static char *tokstr = NULL;

void get_next_token()
{
	tokstr = strtok(NULL, " ");
	if(sscanf(tokstr, "%d", &tokval) == 1) tokstr = NULL;
	else tokval = 0;
}
void parameter_list(int num_params);
void expression();
void factor()
{
	Symbol *s = NULL;
	
	if(tokval == identsym)
	{
		get_next_token();
		
		// Generate instruction to push const or var value
		if(s = get_symbol(symbol_table, tokstr))
			if(s->type == CONSTANT) 
				emit(LIT, 0, s->val);
			else if (s->type == VARIABLE) 
				emit(LOD, level - s->lvl, s->adr);
			else error(err[21]); // ?
		else {
			printf(" %s ",tokstr); error(err[11]);
		}
		
		get_next_token();
	} 
	else if (tokval == numbersym)
	{
		get_next_token();

		// Generate instruction to push number literal
		emit(LIT, 0, tokval);
		get_next_token();
	}
	else if (tokval == lparentsym)
	{
		get_next_token();
		expression();

		if(tokval != rparentsym) error(err[22]);

		get_next_token();
	}
	else if (tokval == callsym)
	{
		get_next_token();

		if(tokval != identsym) 
			error(err[14]);

		get_next_token();

		if(!(s = get_symbol(symbol_table, tokstr))) 
			error(err[11]);

		// Generate call instruction if ident is procedure
		if(s->type != PROCEDURE) error(err[15]);
		
		get_next_token();
		parameter_list(s->val);

		emit(CAL, level - s->lvl, s->adr);

		// Generate instruction to recover return value
		emit(INC, 0, 1);
	}
	else error(err[23]);
}

void term()
{
	int mulop;
	
	factor();
	
	while(tokval == multsym || tokval == slashsym)
	{
		mulop = tokval;
		get_next_token();
		factor();

		// Generate instruction to operate on top two stack values
		emit(OPR, 0, (mulop == multsym) ? MUL : DIV);
	}
}

void expression()
{
	int addop;

	if (tokval == plussym || tokval == minussym)
	{
		addop = tokval;
		get_next_token();
		term();

		// Generate instruction to negate stacked term
		if(addop == minussym) emit(OPR, 0 , NEG);
	} 
	else term();
	
	while(tokval == plussym || tokval == minussym)
	{
		addop = tokval;
		get_next_token();
		term();

		// Generate instruction to operate on top two stack values
		emit(OPR, 0 , (addop == plussym) ? ADD : SUB);
	}
}

void rel_op()
{
	// Check if relational operator
	if(!(tokval >= eqlsym && tokval <= geqsym)) error(err[20]);
	get_next_token();
}

void condition()
{
	int r;

	if (tokval == oddsym) 
	{
		get_next_token();
		expression();
		emit(OPR, 0, ODD);
	} 
	else 
	{
		expression();
		r = tokval;
		rel_op();
		expression();
		emit(OPR, 0, EQL + (r - eqlsym));
	}
}

int parameter_block()
{
	int addr = 4;

	if(tokval != lparentsym) 
		error("Procedure must have parameters.");

	get_next_token();

	if(tokval == identsym)
	{
		get_next_token();
		add_symbol(symbol_table, VARIABLE, tokstr, 0, level + 1, addr++);
		get_next_token();

		while(tokval == commasym)
		{
			get_next_token();

			if(tokval != identsym) error("Parameter identifier expected.");

			get_next_token();
			add_symbol(symbol_table, VARIABLE, tokstr, 0, level + 1, addr++);
			get_next_token();
		}
	}

	if(tokval != rparentsym) error(err[0]);
	
	get_next_token();
	return addr;
}

void parameter_list(int num_params)
{
	int params = 0;
	
	if(tokval != lparentsym) 
		error("Missing parameter list at call.");

	get_next_token();

	if(tokval != rparentsym)
	{
		expression();
		params++;
	}

	while(tokval == commasym)
	{
		get_next_token();
		expression();
		params++;
	}

	if(params != num_params) 
		error("Invalid number of parameters in call.");

	while(params > 0){
		emit(STO, 0, stack_size + 4 - 1);
		params--;
	}

	if(tokval != rparentsym) 
		error("Bad calling formating.");

	get_next_token();
}

void statement()
{
	int c1, c2;
	Symbol *s = NULL;

	// Parse an expression and variable assignment
	if(tokval == identsym)
	{ 
		get_next_token();

		if(!(s = get_symbol(symbol_table, tokstr)))
			error(err[11]);

		get_next_token();
		
		if(tokval != becomesym) error(err[13]);

		get_next_token();
		expression();

		if (s->type != VARIABLE) error(err[12]);
		else emit(STO, level - s->lvl, s->adr);
	} 

	// Parse a call statement
	else if (tokval == callsym)
	{
		get_next_token();

		if(tokval != identsym) 
			error(err[14]);

		get_next_token();

		if(!(s = get_symbol(symbol_table, tokstr))) 
			error(err[11]);

		// Generate call instruction if ident is procedure
		if(s->type != PROCEDURE) error(err[15]);

		get_next_token();
		parameter_list(s->val);
		
		emit(CAL, level - s->lvl, s->adr);
	}

	// Parse multiple statements
	else if (tokval == beginsym)
	{ 
		get_next_token();
		statement();

		while (tokval == semicolonsym)
		{
			get_next_token();
			statement();
		}

		if(tokval != endsym) error(err[19]);

		get_next_token();
	} 

	// Parse an if/then/else conditional statement
	else if (tokval == ifsym)
	{
		get_next_token();
		condition();

		if(tokval != thensym) error(err[16]);
		else get_next_token();

		c1 = cx;		  // Store address for a conditional jump instruction
		emit(JPC, 0, 0);  // Generate with null destination
		statement();	  

		if(tokval == elsesym)
		{
			c2 = cx;		   // Store address for a jump instruction
			emit(JMP, 0, 0);   // Generate JMP with a null destination
			code[c1][2] = cx;  // Update JPC to skip to 'else' on false
			get_next_token();
			statement();
			code[c2][2] = cx;  // Update JMP in 'then' to skip over 'else' 
		}
		else code[c1][2] = cx; // Only update JPC to skip over 'then' on false
	} 

	// Parse a while loop
	else if(tokval == whilesym)
	{
		c1 = cx;
		get_next_token();
		condition();
		c2 = cx;
		emit(JPC, 0, 0);

		if(tokval != dosym) error(err[18]);
		else get_next_token();

		statement();
		emit(JMP, 0, c1);
		code[c2][2] = cx;
	}

	// Parse a read function
	else if(tokval == readsym)
	{
		get_next_token();

		if(tokval != identsym) 
			error("Identifier expected after read.");
		
		get_next_token();
		
		if(!(s = get_symbol(symbol_table, tokstr))){
			printf(" %s ",tokstr); error(err[11]);
		}

		// Generate read and store instructions
		emit(SIO, 0, REA);

		if(s->type == VARIABLE)	emit(STO, level - s->lvl, s->adr);
		else error(err[12]);

		get_next_token();
	}

	// Parse a write function
	else if(tokval == writesym)
	{
		get_next_token();
		expression();
		emit(SIO, 0, WRT);
	}
}

void block(int num_locals)
{
	int n, j = cx;
	char *tmp;

	level++;

	emit(JMP, 0, 0);

	// Parse any constant declarations
	if(tokval == constsym)
	{
		do {
			get_next_token();

			if (tokval != identsym) error(err[4]);
			
			get_next_token();
			
			tmp = tokstr;

			get_next_token();
			
			if (tokval == becomesym) error(err[1]);
			else if (tokval != eqlsym) error(err[3]);
			
			get_next_token();
			
			if(tokval != numbersym) error(err[2]);

			get_next_token();

			if(tokstr != NULL) error("Integer expected.");
			
			add_symbol(symbol_table, CONSTANT, tmp, tokval, level, 0);
			get_next_token();

		} while (tokval == commasym);

		if(tokval != semicolonsym) error(err[5]);

		get_next_token();
	}

	// Parse any variable declarations
	if (tokval == varsym)
	{
		do {
			get_next_token();

			if(tokval != identsym) error(err[4]);
			
			get_next_token();
			add_symbol(symbol_table, VARIABLE, tokstr, 0, level, num_locals++);
			get_next_token();

		} while(tokval == commasym);

		if(tokval != semicolonsym) error(err[5]);

		get_next_token();
	}

	// Parse any procedure declarations
	while(tokval == procsym)
	{
		get_next_token();

		if(tokval != identsym) error(err[4]);

		get_next_token();
		tmp = tokstr;
		get_next_token();
		n = parameter_block();
		add_symbol(symbol_table, PROCEDURE, tmp, n - 4, level, cx);

		if(tokval != semicolonsym) error(err[6]);

		get_next_token();

		// Add symbol for implicit return variable scoped for the following block
		add_symbol(symbol_table, VARIABLE, "return", 0, level + 1, 0);
		block(n);

		if(tokval != semicolonsym) error(err[17]);

		get_next_token();
	}

	code[j][2] = cx;

	// Generate local/variable declaration instruction to increment sp
	emit(INC, 0, num_locals); 	
	statement();

	// Generate return instruction
	if(level) emit(OPR, 0, RET);
	else emit(SIO, 0, HLT);

	remove_level(symbol_table, level--);
}

void parse_program()
{
	// Create a symbol table
	if(symbol_table != NULL) 
		destroy_st(symbol_table);

	symbol_table = new_st(50);

	// Get first token
	tokstr = strtok(&lexemeList[strlen("Lexeme List:\n")], " ");
	if(sscanf(tokstr, "%d", &tokval) == 1) tokstr = NULL;
	else tokval = 0;

	// Parse main block
	block(4);
	
	if (tokval != periodsym) error(err[9]);

	// Clean up
	destroy_st(symbol_table);
}