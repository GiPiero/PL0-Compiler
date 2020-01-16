#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#define MAX_NUMBER_LENGTH 5
#define MAX_IDENTIFIER_LENGTH 11

#define MAX_CODE_LENGTH 32768

extern int nulsym, identsym, numbersym, plussym,
minussym, multsym, slashsym, oddsym, eqlsym,
neqsym, lessym, leqsym, gtrsym, geqsym,
lparentsym, rparentsym, commasym, semicolonsym,
periodsym, becomesym, beginsym, endsym, ifsym,
thensym, whilesym, dosym, callsym, constsym,
varsym, procsym, writesym, readsym, elsesym;

extern char lexemeList[MAX_CODE_LENGTH];
extern char symbolicLexemeList[MAX_CODE_LENGTH];

extern FILE * outFile;

void openFiles(char * inputFile, char * outputFile);
void echoInput();
void processText();

#endif