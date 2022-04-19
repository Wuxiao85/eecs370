/**
 * Project 1 
 * Assembler code fragment for LC-2K 
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int checkLabel(char *);
int CalcReg(int, char);
int CalcOffset(char *, char *, FILE *, int, char[][7], int, int[]);


int
main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    struct global {
        char symbol[7];
        char type[2];
        int offset;
    };

    struct global globals[50];

    struct relocate {
        int offset;
        char opcode[7];
        char symbol[7];
    };

    struct relocate relocations[50];
    int num_relocates = 0;

    char labels[50][7];  // Array of labels
    int label_addr[50];
    int num_labels = 0;
    int num_text = 0, num_data = 0, num_symbols = 0;

    int h, i, j, k, data_start = -1;
    for (k = 0, i = 0, h = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); ++k) {
        if (!strcmp(opcode, ".fill") && data_start < 0) {
            data_start = k;
        }

        if (checkLabel(label)) {                // If label found
            for (int x = 0; x < num_labels; ++x) {
                if (!strcmp( labels[x], label )) {    // Check for duplicate labels
                    printf("error : duplicate labels detected\n");
                    exit(1);
                }
            }

            label_addr[i] = k;
            strcpy(labels[i++], label);
            ++num_labels;

            if (isupper(label[0])) {                    // If label is global
                strcpy(globals[h].symbol, label);       // add to array of globals

                ++num_symbols;                          // Symbols

                if (!strcmp(opcode, ".fill")) {
                   strcpy(globals[h].type, "D");        // Symbol is defined as data
                    globals[h++].offset = k - data_start;
                }
                else {
                    strcpy(globals[h].type, "T");       // Symbol is defined as text
                    globals[h++].offset = k;
                }
            }
        }

        if (!strcmp(opcode, ".fill")) {
            ++num_data;            // Data
        }
        else if (!strcmp(opcode, "add") || !strcmp(opcode, "nor")
            || !strcmp(opcode, "lw") || !strcmp(opcode, "sw") 
            || !strcmp(opcode, "beq") || !strcmp(opcode, "jalr") 
            || !strcmp(opcode, "noop") || !strcmp(opcode, "halt")) {
            ++num_text;            // Text
        }
        else {
            printf("error : unrecognized opcode\n");
            exit(1);
        }
    }

    rewind(inFilePtr);

    for (k = 0, j = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); ++k) {

        if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw")) {
            if (!isNumber(arg2)) {      // If symbolic address was used in lw or sw...

                relocations[j].offset = k;                  // Add symbolic address to array of relocations
                strcpy(relocations[j].opcode, opcode);
                strcpy(relocations[j++].symbol, arg2);

                ++num_relocates;        // Relocations

                if (isupper(arg2[0])) {
                    int found = 0;
                    for (int i = 0; i < num_labels; ++i) {
                        if (!strcmp(labels[i], arg2)) {     // If global was already defined...
                            found = 1;
                        }
                    }

                    for (int i = 0; i < num_symbols; ++i) {
                        if (!strcmp(globals[i].symbol, arg2)) {     // If global was already discovered...
                            found = 1;
                        }
                    }

                    if (!found) {
                        ++num_symbols;  // Symbols
                        strcpy(globals[h].symbol, arg2);    // Add undef globals to array if used in lw or sw
                        strcpy(globals[h].type, "U");       // Symbolic address was undefined
                        globals[h++].offset = 0;
                    }
                }
            }
        }
        else if (!strcmp(opcode, "beq")) {
            if (!isNumber(arg2) && isupper(arg2[0])) {  // If symbolic label was used in beq...
                int found = 0;
                for (int i = 0; i < num_labels; ++i) {
                    if (!strcmp(labels[i], arg2)) {     // If global was already defined...
                        found = 1;
                    }
                }

                if (!found) {
                    printf("error : 'beq' using an undefined global symbolic address\n");
                    exit(1);
                }
                
            }
        }
        else if (!strcmp(opcode, ".fill")) {
            if (!isNumber(arg0)) {      // If symbolic address was used in .fill...
                relocations[j].offset = k - data_start;     // Add symbolic address to array of relocations
                strcpy(relocations[j].opcode, opcode);
                strcpy(relocations[j++].symbol, arg0);
                ++num_relocates;        // Relocations

                if (isupper(arg0[0])) {
                    int found = 0;
                    for (int i = 0; i < num_labels; ++i) {
                        if (!strcmp(labels[i], arg0)) {     // If global was already defined...
                            found = 1;
                        }
                    }

                    for (int i = 0; i < num_symbols; ++i) {
                        if (!strcmp(globals[i].symbol, arg0)) {     // If global was already discovered...
                            found = 1;
                        }
                    }

                    if (!found) {
                        ++num_symbols;  // Symbols
                        strcpy(globals[h].symbol, arg0);    // Add undef globals to array if used in lw or sw
                        strcpy(globals[h].type, "U");       // Symbolic address was undefined
                        globals[h++].offset = 0;
                    }
                }
            }
        }
    }

    // HEADER
    fprintf(outFilePtr, "%d %d %d %d\n", num_text, num_data, num_symbols, num_relocates);

    rewind(inFilePtr);
    
    // TEXT & DATA
    int PC, mc;
    for (mc = 0, PC = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); ++PC, mc = 0) {
        // opcode
        if (!strcmp(opcode, "add")) {       // add (000)
            mc += CalcReg(atoi(arg0), 'A') + CalcReg(atoi(arg1), 'B') + CalcReg(atoi(arg2), 'C');
        }
        else if (!strcmp(opcode, "nor")) {  // nor (001)
            mc = (int)(pow(2, 22)+0.5);
            mc += CalcReg(atoi(arg0), 'A') + CalcReg(atoi(arg1), 'B') + CalcReg(atoi(arg2), 'C');
        }
        else if (!strcmp(opcode, "lw")) {   // lw (010)
            mc = (int)(pow(2, 23)+0.5);
            mc += CalcReg(atoi(arg0), 'A') + CalcReg(atoi(arg1), 'B');
            mc += CalcOffset(opcode, arg2, inFilePtr, PC, labels, num_labels, label_addr);
        }
        else if (!strcmp(opcode, "sw")) {   // sw (011)
            mc = (int)(pow(2, 23)+0.5) + (int)(pow(2, 22)+0.5);
            mc += CalcReg(atoi(arg0), 'A') + CalcReg(atoi(arg1), 'B');
            mc += CalcOffset(opcode, arg2, inFilePtr, PC, labels, num_labels, label_addr);
        }
        else if (!strcmp(opcode, "beq")) {  // beq (100)
            mc = (int)(pow(2, 24)+0.5);
            mc += CalcReg(atoi(arg0), 'A') + CalcReg(atoi(arg1), 'B');
            mc += CalcOffset(opcode, arg2, inFilePtr, PC+1, labels, num_labels, label_addr);
        }
        else if (!strcmp(opcode, "jalr")) { // jalr (101)
            mc = (int)(pow(2, 24)+0.5) + (int)(pow(2, 22)+0.5);
            mc += CalcReg(atoi(arg0), 'A') + CalcReg(atoi(arg1), 'B');
        }
        else if (!strcmp(opcode, "halt")) { // halt (110)
            mc = (int)(pow(2, 24)+0.5) + (int)(pow(2, 23)+0.5);
        }
        else if (!strcmp(opcode, "noop")) { // noop (111)
            mc = (int)(pow(2, 24)+0.5) + (int)(pow(2, 23)+0.5) + (int)(pow(2, 22)+0.5);
        }
        else if (!strcmp(opcode, ".fill")) { // .fill
            mc = CalcOffset(opcode, arg0, inFilePtr, 0, labels, num_labels, label_addr);
        }

        fprintf(outFilePtr, "%d\n", mc);
    }

    // SYMBOL
    for (int i = 0; i < num_symbols; i++) {
        fprintf(outFilePtr, "%s %s %d\n", globals[i].symbol, globals[i].type, globals[i].offset);
    }

    // RELOCATION
    for (int i = 0; i < num_relocates; i++) {
        fprintf(outFilePtr, "%d %s %s\n", relocations[i].offset, relocations[i].opcode, relocations[i].symbol);
    }

    exit(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if successfully read
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
        char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* reached end of file */
        return(0);
    }

    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }

    /* is there a label? */
    char *ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int
isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

int
CalcReg(int n, char ABC)
{
    int MSB = 0;
    if (ABC == 'A')
        MSB = 21;   // ABC == A then (bits 21-19)
    if (ABC == 'B')
        MSB = 18;   // ABC == B then (bits 18-16)
    if (ABC == 'C')
        MSB = 2;    // ABC == C then (bits 2-0)

    int x = 0;
    if (n / 4 == 1) {
        x += (int)(pow(2, MSB)+0.5);
        n -= 4;
    }
    if (n / 2 == 1) {
        x += (int)(pow(2, (MSB - 1.0))+0.5);
        n -= 2;
    }
    if (n - 1 == 0) {
        x += (int)(pow(2, (MSB - 2.0))+0.5);
        n -= 1;
    }
    return x;
}

int
CalcOffset(char* opcode, char* arg, FILE* inFile, int PC, char labels[][7], int n_labels, int label_addr[])
{
    if (isNumber(arg) && (atoi(arg) < -32768 || atoi(arg) > 32767)) {
        printf("error : offsetField does not fit in 16 bits\n");
        exit(1);
    }

    if (isNumber(arg)) {
        if (atoi(arg) >= 0 || !strcmp(opcode, ".fill"))
            return atoi(arg);
        else
            return (65535 + atoi(arg) + 1);
    }
    else {
        int i;
        for (i = 0; i < n_labels; ++i) {    // Find symbolic address in array of labels
            if (!strcmp(arg, labels[i])) {   // If found...
                if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw"))
                    return label_addr[i];
                else {
                    int offset = 0;
                    offset = label_addr[i] - PC;    // Return offset = Address of label - PC
                    if (offset < 0) {
                        return (65535 + offset + 1);
                    }
                    else
                        return offset;
                }
            }
        }

        // If symbolic label not defined...
        if (isupper(arg[0]) && (!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, ".fill")))
                return 0;
        
        printf("error : use of undefined symbolic address\n");
        exit(1);
    }
}

int
checkLabel(char* label)
{
    if (!strcmp(label, "")) {
        return 0;
    }
    else
        return 1;
}
