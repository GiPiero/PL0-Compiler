driver : compiler.o parsegen.o symboltable.o lexicalAnalyzer.o vm.o
	gcc -o driver compiler.o parsegen.o symboltable.o lexicalAnalyzer.o vm.o

compiler.o : compiler.c lexicalAnalyzer.h parsegen.h symboltable.h vm.h
	gcc -c compiler.c

parsegen.o : parsegen.c parsegen.h lexicalAnalyzer.h symboltable.h
	gcc -c parsegen.c

symboltable.o : symboltable.c symboltable.h
	gcc -c symboltable.c -lm

lexicalAnalyzer.o : lexicalAnalyzer.c lexicalAnalyzer.h
	gcc -c lexicalAnalyzer.c

vm.o : vm.c vm.h
	gcc -c vm.c

clean :
	rm driver compiler.o parsegen.o symboltable.o lexicalAnalyzer.o vm.o