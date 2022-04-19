/**
 * Project 2
 * LC-2K Linker
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAXSIZE 300
#define MAXLINELENGTH 1000
#define MAXFILES 6

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

struct SymbolTableEntry {
	char label[7];
	char location;
	int offset;
};

struct RelocationTableEntry {
	int offset;
	char inst[7];
	char label[7];
	int file;
};

struct FileData {
	int textSize;
	int dataSize;
	int symbolTableSize;
	int relocationTableSize;
	int textStartingLine; // in final executible
	int dataStartingLine; // in final executible
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry symbolTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles {
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry     symTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
	int textSize;
	int dataSize;
	int symTableSize;
	int relocTableSize;
};

void checkLabel(CombinedFiles, char*, char);
void relocateText(int, struct CombinedFiles*, FileData[], int, int);
void relocateData(int, struct CombinedFiles*, FileData[], int, int);
int linkGlobal(char*, int, FileData files[], struct CombinedFiles*, int, int);

int main(int argc, char *argv[])
{
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr; 
	int i, j;

	if (argc <= 2) {
		printf("error: usage: %s <obj file> ... <output-exe-file>\n",
				argv[0]);
		exit(1);
	}

	outFileString = argv[argc - 1];

	outFilePtr = fopen(outFileString, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileString);
		exit(1);
	}

	FileData files[MAXFILES];

	//Reads in all files and combines into master
	for (i = 0; i < argc - 2; i++) {
		inFileString = argv[i+1];

		inFilePtr = fopen(inFileString, "r");
		printf("opening %s\n", inFileString);

		if (inFilePtr == NULL) {
			printf("error in opening %s\n", inFileString);
			exit(1);
		}

		char line[MAXLINELENGTH];
		int sizeText, sizeData, sizeSymbol, sizeReloc;

		// parse first line
		fgets(line, MAXSIZE, inFilePtr);
		sscanf(line, "%d %d %d %d",
				&sizeText, &sizeData, &sizeSymbol, &sizeReloc);

		files[i].textSize = sizeText;
		files[i].dataSize = sizeData;
		files[i].symbolTableSize = sizeSymbol;
		files[i].relocationTableSize = sizeReloc;

		// read in text
		int instr;
		for (j = 0; j < sizeText; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			instr = atoi(line);
			files[i].text[j] = instr;
		}

		// read in data
		int data;
		for (j = 0; j < sizeData; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			data = atoi(line);
			files[i].data[j] = data;
		}

		// read in the symbol table
		char label[7];
		char type;
		int addr;
		for (j = 0; j < sizeSymbol; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%s %c %d",
					label, &type, &addr);
			files[i].symbolTable[j].offset = addr;
			strcpy(files[i].symbolTable[j].label, label);
			files[i].symbolTable[j].location = type;
		}

		// read in relocation table
		char opcode[7];
		for (j = 0; j < sizeReloc; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%d %s %s",
					&addr, opcode, label);
			files[i].relocTable[j].offset = addr;
			strcpy(files[i].relocTable[j].inst, opcode);
			strcpy(files[i].relocTable[j].label, label);
			files[i].relocTable[j].file	= i;
		}
		fclose(inFilePtr);
	} // end reading files

	// *** INSERT YOUR CODE BELOW ***
	//    Begin the linking process
	//    Happy coding!!!

	CombinedFiles linked;
	linked.textSize = 0;
	linked.dataSize = 0;
	linked.symTableSize = 0;
	linked.relocTableSize = 0;

	for (int i = 0, f = 0; f < argc - 2; ++f) {
		for (int j = 0; j < files[f].symbolTableSize; ++j) {
			if (files[f].symbolTable[j].location != 'U' && !strcmp("Stack", files[f].symbolTable[j].label)) {
				printf("error : 'Stack' label cannot be defined by an object file.\n");
				exit(1);
			}
			checkLabel(linked, files[f].symbolTable[j].label, files[f].symbolTable[j].location);
			strcpy(linked.symTable[i].label, files[f].symbolTable[j].label);
			linked.symTable[i].location = files[f].symbolTable[j].location;
			linked.symTable[i].offset = files[f].symbolTable[j].offset;
			++linked.symTableSize;
			++i;
		}
	}

	for (int i = 0, f = 0; f < argc - 2; ++f) {
		for (int j = 0; j < files[f].relocationTableSize; ++j) {
			linked.relocTable[i].offset = files[f].relocTable[j].offset;
			strcpy(linked.relocTable[i].inst, files[f].relocTable[j].inst);
			strcpy(linked.relocTable[i].label, files[f].relocTable[j].label);
			linked.relocTable[i].file = f;
			++linked.relocTableSize;
			++i;
		}
	}

	for (int i = 0, f = 0; f < argc - 2; ++f) {
		files[f].textStartingLine = i;
		for (int j = 0; j < files[f].textSize; ++j, ++i) {
			linked.text[i] = files[f].text[j];
			++linked.textSize;
		}
	}
	
	for (int i = 0, f = 0; f < argc - 2; ++f) {
		files[f].dataStartingLine = i + linked.textSize;
		for (int j = 0; j < files[f].dataSize; ++j, ++i) {
			linked.data[i] = files[f].data[j];
			++linked.dataSize;
		}
	}

	strcpy(linked.symTable[linked.symTableSize].label, "Stack");
	linked.symTable[linked.symTableSize].offset = linked.textSize + linked.dataSize;
	linked.symTable[linked.symTableSize].location = 'G';
	++linked.symTableSize;

	for (int ln = 0, f = 0; f < argc - 2; ++f) {
		if (f == argc - 3) {
			// Last file text segment
			for (; ln < linked.textSize; ++ln) {
				relocateText(ln, &linked, files, f, argc);
			}
			// done converting text
		}
		else {
			for (; ln < files[f + 1].textStartingLine; ++ln) {
				// Relocate any symbols for line
				relocateText(ln, &linked, files, f, argc);
			}
		}
	}

	for (int ln = linked.textSize, f = 0; f < argc - 2; ++f) {
		if (f == argc - 3) {
			// Last file data segment
			for (; ln < linked.dataSize + linked.textSize; ++ln) {
				relocateData(ln, &linked, files, f, argc);
			}
			// done converting data
		}
		else {
			for (; ln < files[f + 1].dataStartingLine; ++ln) {
				// Relocate any symbols for line
				relocateData(ln, &linked, files, f, argc);
			}
		}
	}

	// test
	printf("%d %d %d %d\n", linked.textSize, linked.dataSize, linked.symTableSize, linked.relocTableSize);

	for (int j = 0; j < linked.textSize; ++j) {
		fprintf(outFilePtr, "%d\n", linked.text[j]);
	}

	for (int j = 0; j < linked.dataSize; ++j) {
		fprintf(outFilePtr, "%d\n", linked.data[j]);
	}

	// test
	for (int j = 0; j < linked.symTableSize; ++j) {
		printf("%s %c %d\n", linked.symTable[j].label, linked.symTable[j].location, linked.symTable[j].offset);
	}

	for (int j = 0; j < linked.relocTableSize; ++j) {
		printf("%d %s %s < file[%d]\n", 
			linked.relocTable[j].offset, linked.relocTable[j].inst, linked.relocTable[j].label, linked.relocTable[j].file);
	}
	
} // end main

void checkLabel(CombinedFiles linked, char* label, char location) {
	for (int i = 0; i < linked.symTableSize; ++i) {
		if (!strcmp(linked.symTable[i].label, label)) {
			if (location != 'U' && linked.symTable[i].location != 'U') {
				printf("error : duplicate defined global labels.\n");
				exit(1);			
			}
		}
	}
}

void relocateText(int line, struct CombinedFiles *linked, FileData files[], int f, int argc) {
	// Relocate any symbols for line
	int n = line - files[f].textStartingLine;
	for (int j = 0; j < linked->relocTableSize; ++j) {
		
		// If relocation needed at line...
		if (linked->relocTable[j].offset == n && 
			linked->relocTable[j].file == f &&
			strcmp(".fill", linked->relocTable[j].inst)) {
			// If global symbol...
			if (isupper(linked->relocTable[j].label[0])) {
				
				if (!strcmp("Stack", linked->relocTable[j].label)) {
					linked->text[line] += linked->symTable[linked->symTableSize - 1].offset;
				}
				else
					linked->text[line] += linkGlobal(linked->relocTable[j].label, line, files, &linked, argc, f);
			}
			else {	// If local var...
				//linked->text[line] -= files[f].textSize;

				if ((uint16_t)linked->text[line] >= (uint16_t)files[f].textSize) {	// local defined in data
					linked->text[line] += files[f].textStartingLine;
					for (int k = f + 1; k < argc - 2; ++k) {
						linked->text[line] += files[k].textSize;
					}

					for (int k = 0; k != f; ++k) {
						linked->text[line] += files[k].dataSize;
					}
				}
				else {	// local defined in text
					linked->text[line] += files[f].textStartingLine;
				}
			}
			
		}
	}
}

void relocateData(int line, struct CombinedFiles* linked, FileData files[], int f, int argc) {
	// Relocate any symbols for line
	int n = line - files[f].dataStartingLine;
	int i = line - linked->textSize;
	for (int j = 0; j < linked->relocTableSize; ++j) {
		// If relocation needed at line...
		if (linked->relocTable[j].offset == n && 
			linked->relocTable[j].file == f &&
			!strcmp(".fill", linked->relocTable[j].inst)) {

			// If global symbol...
			if (isupper(linked->relocTable[j].label[0])) {

				if (!strcmp("Stack", linked->relocTable[j].label)) {
					linked->data[i] += linked->symTable[linked->symTableSize - 1].offset;
				}
				else
					linked->data[i] += linkGlobal(linked->relocTable[j].label, line, files, &linked, argc, f);

			}
			else {	// If local var...
				if ((uint16_t)linked->data[i] >= (uint16_t)files[f].textSize) {	// local defined in data
					linked->data[i] += files[f].textStartingLine;
					for (int k = f + 1; k < argc - 2; ++k) {
						linked->data[i] += files[k].textSize;
					}

					for (int k = 0; k != f; ++k) {
						linked->data[i] += files[k].dataSize;
					}
				}
				else {	// local defined in text
					linked->data[i] += files[f].textStartingLine;
				}
			}
		}
	}
}

int linkGlobal(char* label, int line, FileData files[], struct CombinedFiles *linked, int argc, int f) {
	// Search in symbol table
	int offset = -1;
	
	for (int f1 = 0; f1 < argc - 2; ++f1) {
		for (int j1 = 0; j1 < files[f1].symbolTableSize; ++j1) {
			if (!strcmp(label, files[f1].symbolTable[j1].label)) {	

				if (f1 != f) {
					if (files[f1].symbolTable[j1].location == 'T') {
						offset = files[f1].textStartingLine + files[f1].symbolTable[j1].offset;
					}
					else if (files[f1].symbolTable[j1].location == 'D') {
						offset = files[f1].dataStartingLine + files[f1].symbolTable[j1].offset;
					}
				}
				else {
					if (files[f1].symbolTable[j1].location == 'T') {
						offset = 0;
						offset -= files[f1].symbolTable[j1].offset;
						offset += (files[f1].textStartingLine + files[f1].symbolTable[j1].offset);
					}
					else if (files[f1].symbolTable[j1].location == 'D') {
						offset = 0;
						offset -= (files[f1].textSize + files[f1].symbolTable[j1].offset);
						offset += (files[f1].dataStartingLine + files[f1].symbolTable[j1].offset);
					}
				}
			}
		}
	}

	if (offset < 0) {
		printf("error : undefined global label.\n");
		exit(1);
	}
	
	return offset;
}