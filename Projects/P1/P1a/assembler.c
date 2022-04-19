/**
 * Project 1 
 * Assembler code fragment for LC-2K 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int checkLabel(char *);
int CalcReg(int, char);
int CalcOffset(char *, char *, FILE *, int, char[][7], int, int[]);
//void CheckReg(char*, char*);


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

    ///* here is an example for how to use readAndParse to read a line from
    //    inFilePtr */
    //if (! readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
    //    /* reached end of file */
    //}

    ///* this is how to rewind the file ptr so that you start reading from the
    //    beginning of the file */
    //rewind(inFilePtr);

    ///* after doing a readAndParse, you may want to do the following to test the
    //    opcode */
    //if (!strcmp(opcode, "add")) {
    //    /* do whatever you need to do for opcode "add" */
    //}

    char labels[100][7];  // Array of labels
    int label_addr[100];
    int num_labels = 0;

    int k;
    int i;
    for (k = 0, i = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); ++k) {
        if (checkLabel(label)) {                // If label found
            if (k > 0) {                // passed line 0
                int j;
                for (j = 0; j < i; ++j) {
                    if (!strcmp( labels[j], label )) {    // Check for duplicate labels
                        printf("error : duplicate labels detected\n");
                        exit(1);
                    }
                }
            }

            label_addr[i] = k;
            strcpy(labels[i], label);
            //printf("%i :", i);
            //printf(" %s in address %i\n", labels[i], label_addr[i]);
            ++i;
            ++num_labels;
        }
    }

    rewind(inFilePtr);
    
    int PC;
    for (PC = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); ++PC) {
        int mc = 0;
        /*fprintf(outFilePtr, "(address ");
        fprintf(outFilePtr, "%d", PC);*/

        // opcode
        if (!strcmp(opcode, "add")) {   // add (000)
            //CheckReg(arg0, arg1);
            mc += CalcReg(atoi(arg0), 'A') + CalcReg(atoi(arg1), 'B') + CalcReg(atoi(arg2), 'C');
        }
        else if (!strcmp(opcode, "nor")) {  // nor (001)
            //CheckReg(arg0, arg1);
            mc = (int)(pow(2, 22)+0.5);
            mc += CalcReg(atoi(arg0), 'A') + CalcReg(atoi(arg1), 'B') + CalcReg(atoi(arg2), 'C');
        }
        else if (!strcmp(opcode, "lw")) {   // lw (010)
            //CheckReg(arg0, arg1);
            mc = (int)(pow(2, 23)+0.5);
            mc += CalcReg(atoi(arg0), 'A') + CalcReg(atoi(arg1), 'B');
            mc += CalcOffset(opcode, arg2, inFilePtr, PC, labels, num_labels, label_addr);
        }
        else if (!strcmp(opcode, "sw")) {   // sw (011)
            //CheckReg(arg0, arg1);
            mc = (int)(pow(2, 23)+0.5) + (int)(pow(2, 22)+0.5);
            mc += CalcReg(atoi(arg0), 'A') + CalcReg(atoi(arg1), 'B');
            mc += CalcOffset(opcode, arg2, inFilePtr, PC, labels, num_labels, label_addr);
        }
        else if (!strcmp(opcode, "beq")) {  // beq (100)
            //CheckReg(arg0, arg1);
            mc = (int)(pow(2, 24)+0.5);
            mc += CalcReg(atoi(arg0), 'A') + CalcReg(atoi(arg1), 'B');
            mc += CalcOffset(opcode, arg2, inFilePtr, PC+1, labels, num_labels, label_addr);
        }
        else if (!strcmp(opcode, "jalr")) { // jalr (101)
            //CheckReg(arg0, arg1);
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
        else {
            printf("error : unrecognized opcode\n");
            exit(1);
        }

        fprintf(outFilePtr, "%d\n", mc);
        /*fprintf(outFilePtr, " (hex 0x%x", mc);
        fprintf(outFilePtr, ")\n");*/
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

//void
//CheckReg(char* arg0, char* arg1)
//{
//    if (!isNumber(arg0)) {
//        printf("error : arg0 is not an integer\n");
//        exit(1);
//    }
//    if (!isNumber(arg1)) {
//        printf("error : arg1 is not an integer\n");
//        exit(1);
//    }
//    if (atoi(arg0) < 0 || 7 < atoi(arg0) || atoi(arg1) < 0 || 7 < atoi(arg1)) {
//        printf("error : arg0/arg1 needs to be an integer from 0 to 7\n");
//        exit(1);
//    }
//}

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
        // If not found...
        printf("error : use of undefined label\n");
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
