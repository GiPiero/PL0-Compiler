#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "vm.h"
#include "parsegen.h"
#include "lexicalAnalyzer.h"

#define PRINT_INPUT 1

enum flags {L = 1, A = 2, V = 4};

int main(int argc, char **argv)
{
	int i, c;
	char flags = 0; 
	FILE *code_file;
	unsigned long file_pos;

 	if (argc > 4) 
 	{
 		printf("Invalid number arguments for compiler!\n");
 		return 0;
 	}

 	for(i = 1; i < argc; i++) 
 	{
 		if(strcmp(argv[i], "-l") == 0) flags |= L;
 		else if(strcmp(argv[i], "-a") == 0) flags |= A;
 		else if(strcmp(argv[i], "-v") == 0) flags |= V;
 		else printf("Invalid argument: %s\n", argv[i]);
 	}

	// Remove previous error file if it exists
	remove("ef");

	// Scan in lexemes
	openFiles("in.txt", "out.txt");
	echoInput();
	processText();

	// Print scanned lexemes to screen
	if(flags & L) 
		printf("%s\n\n%s\n\n", lexemeList, symbolicLexemeList);

	// Parse and generate assembly
	parse_program();

	code_file = fopen("vminput.txt", "w+");
	print_assembly(code_file);

	printf("No errors, program is syntactically correct.\n\n");

	// Print generated assembly to screen
	if(flags & A)
	{
		printf("Generated assembly:\n");
		print_assembly(stdout);
		printf("\n");
	}

	// Scan generated assembly into VM
	rewind(code_file);
	if(!read_input(code_file)) return 0;

	// Print VM instructions
	fprintf(outFile, "\n\n");
	print_input(outFile);
	
	if(flags & V)
	{
		printf("VM Instructions:\n");
		print_input(stdout);
	}

	// Remember position of VM output in outFile
	fflush(outFile);
	file_pos = ftell(outFile);
	
	// Execute compiled program
	printf("Program execution:\n");
	fetch_and_execute(outFile);
	
	// Print VM output
	fclose(outFile);
	outFile = fopen("out.txt", "r");
	
	fseek(outFile, file_pos, SEEK_SET);

	if(flags & V)
		while((c = getc(outFile)) != EOF) putchar(c);

	// Clean up
	fclose(outFile);
	fclose(code_file);

	return 1;
}