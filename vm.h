#ifndef VM_H
#define VM_H

#define MAX_STACK_HEIGHT 2000
#define MAX_INST_COUNT 32768
//#define MAX_LEXI_LEVELS 3

int read_input(FILE *in);
void print_input(FILE *out);
void fetch_and_execute(FILE *out);


#endif 