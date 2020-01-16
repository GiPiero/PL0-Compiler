#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

#define BUFFLEN 50

typedef struct instruction {
	unsigned op;
	int l;
	int m;
} inst;

/* CPU Registers */
static unsigned bp = 1;
static unsigned sp = 0;
static unsigned pc = 0;
static inst ir;

/* Memory Stores */
static int code_len = 0;
static inst code[MAX_INST_COUNT];
static int stack[MAX_STACK_HEIGHT];

static int top_ari = 0;
static int display[MAX_STACK_HEIGHT / 4];

/* Flags */
static int run = 1;

/* Helper functions */
int base(int lex, int base) 
{
	int b;
	for(b = base; lex > 0; lex--) b = stack[b + 1];
	return b;
}

/* Read/Write functions */
int read_input(FILE *fp)
{
	char buff[BUFFLEN];
	int i = 0;

	// Count the number of instructions
	while(!feof(fp)) 
	{
		if(fgetc(fp) == '\n')
		{
			if(++code_len >= MAX_INST_COUNT) 
			{
				fprintf(stderr, "Error: Instruction count exceeds %d\n", MAX_INST_COUNT);
				return 0;
			}
		}
	}

	// Scan in each instruction
	for(rewind(fp); fgets(buff, BUFFLEN, fp); i++) 
	{
		if(sscanf(buff, "%d %d %d", &code[i].op, &code[i].l, &code[i].m) != 3) 
		{
			fprintf(stderr, "Error: Could not read line %d.\n", i);
			return 0;
		}

		if(code[i].op > 9 || code[i].op < 1)
		{
			fprintf(stderr, "Error: Invalid op code '%d' on line %d\n", code[i].op, i);
			return 0;
		}
		else if(code[i].op == 2 && code[i].m > 13)
		{
			fprintf(stderr, "Error: Invalid OPR instruction '%d' on line %d.\n", ir.m, pc);
			return 0;
		}
	}
	return 1;
}

static const char * const opsym[9] = { 
	"lit", "opr", "lod",
	"sto", "cal", "inc",
	"jmp", "jpc", "sio" 
};

void print_input(FILE *out)
{
	int i;

	fprintf(out, "%-8s%-8s%-8s%s\n","Line","OP","L","M");

	for(i = 0; i < code_len; i++)
		fprintf(out, "%-8d%-8s%-8d%d\n", i, opsym[code[i].op-1], code[i].l, code[i].m);
	fprintf(out, "\n");
}

void print_initial_state(FILE *out)
{
	if(!out) return;
	fprintf(out, "%70s", "pc      bp      sp      stack\n");
	fprintf(out, "%-40s%-8d%-8d%-8d\n","Initial Values",pc,bp,sp);
}

void print_state(FILE *out)
{
	int i, ari = 0;

	if(!out) return;

	// Print register values
	fprintf(out, "%-8d%-8d%-8d", pc, bp, sp);

	// Print stack
	for(i = 1; i <= sp; i++) 
	{
		fprintf(out, "%d ", stack[i]);

		if(i + 1 == display[ari])
		{
			fprintf(out, "| ");
			ari++;
		}
	}

	// Print new AR if just created
	if(sp && (bp - sp == 1)) 
		fprintf(out, "%d %d %d %d ", stack[i], stack[i + 1], stack[i+2], stack[i+3]);

	fprintf(out, "\n");
}

/* Arithmetic/Logical functions */
void neg(){stack[sp] = -stack[sp];}
void add(){stack[--sp] += stack[sp+1];}
void sub(){stack[--sp] -= stack[sp+1];}
void mul(){stack[--sp] *= stack[sp+1];}
void dvd(){stack[--sp] /= stack[sp+1];}
void odd(){stack[sp] %= 2;}
void mod(){stack[--sp] %= stack[sp+1];}
void eql(){stack[--sp] = (stack[sp] == stack[sp+1])? 1:0;}
void neq(){stack[--sp] = (stack[sp] != stack[sp+1])? 1:0;}
void lss(){stack[--sp] = (stack[sp] < stack[sp+1])? 1:0;}
void leq(){stack[--sp] = (stack[sp] <= stack[sp+1])? 1:0;}
void gtr(){stack[--sp] = (stack[sp] > stack[sp+1])? 1:0;}
void geq(){stack[--sp] = (stack[sp] >= stack[sp+1])? 1:0;}
void ret()
{
	sp = bp - 1;
	pc = stack[sp + 4];
	bp = stack[sp + 3];
	display[--top_ari] = 0;
}

/* P-Machine execution step */
int execute()
{
	// Arithmetic/Logical jump table
	static void (* const opr_table[])(void) = { 
		ret, neg, add, sub, mul, dvd, odd, 
		mod, eql, neq, lss, leq, gtr, geq 
	};
	
	switch(ir.op)
	{
		case 1: // LIT
			stack[++sp] = ir.m;
			break;
		case 2: // OPR
			opr_table[ir.m]();
			break;
		case 3: // LOD
			stack[++sp] = stack[base(ir.l, bp) + ir.m];
			break;
		case 4: // STO
			stack[base(ir.l, bp) + ir.m] = stack[sp--];
			break;
		case 5: // CAL
			display[top_ari++] = sp + 1;
			stack[sp + 1] = 0;				// Return value
			stack[sp + 2] = base(ir.l, bp);	// Static link (parent AR)
			stack[sp + 3] = bp;				// Dynamic Link (previous AR)
			stack[sp + 4] = pc;				// Return addr (next code index)
			bp = sp + 1;
			pc = ir.m;
			break;
		case 6: // INC
			sp = sp + ir.m;
			break;
		case 7: // JMP
			pc = ir.m;
			break; 
		case 8: // JPC
			if(stack[sp--] == 0) pc = ir.m;
			break;
		case 9:	// SIO
			if(ir.m == 1) // WRITE
			{
				printf("%d\n", stack[sp--]);
			}
			else if(ir.m == 2) // READ
			{
				printf("Input an integer value: ");
				scanf("%d", &stack[++sp]);
			} 
			else if(ir.m == 3) // HALT
			{
				pc = 0;
				bp = 0;
				sp = 0;
				return 0;
			} 
			break;
	}
	return 1;
}

void fetch_and_execute(FILE *out)
{
	// Print initial state
	print_initial_state(out);

	while(run)
	{
		// Fetch instruction
		if(pc < code_len) ir = code[pc++];
		else break;

		// Print Instruction
		if(out) 
			fprintf(out, "%-8d%-8s%-8d%-16d", pc - 1, opsym[ir.op - 1], ir.l, ir.m);

		// Execute
		run = execute();

		print_state(out);
	}
}

// int main(int argc, char **argv)
// {
// 	FILE *fp;

// 	// Open input file stream
// 	if((fp = fopen("vminput.txt", "r")) == NULL)
// 	{
// 		fprintf(stderr, "Error: Could not open vminput.txt\n");
// 		return 0;
// 	}

// 	// Read input and open new output file stream
// 	if(run = read_input(fp))
// 	{
// 		fclose(fp);
// 		if((fp = fopen("vmoutput.txt", "w")) == NULL){
// 			fprintf(stderr, "Error: Could not open vminput.txt\n");
// 			return 0;
// 		}
// 	}

// 	print_input(fp);

// 	// Fetch-execute cycle
// 	fetch_and_execute(fp);

// 	fclose(fp);
// 	return 1;
// }