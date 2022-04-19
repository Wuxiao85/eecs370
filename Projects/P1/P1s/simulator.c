/**
 * Project 1
 * EECS 370 LC-2K Instruction-level simulator
 */

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
//#include <cmath>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
int convertNum(int);
int getRegA(int);
int getRegB(int);
int getRegDest(uint8_t);
int getOffset(int);

int
main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
            state.numMemory++) {

        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    int halt = 0;
    int numInstruct = 0;

    for (int i = 0; i < NUMREGS; ++i) {

        state.reg[i] = 0;
    }

    printf("\n");

    for (state.pc = 0; !halt; ++state.pc, ++numInstruct) {
        printState(&state);

        int mc = state.mem[state.pc];
        int op = mc >> 22;
        int regA = getRegA(mc);
        int regB = getRegB(mc);
        int destReg = getRegDest(mc);
        int offset = getOffset(mc);

        if (op == 0) {  // add
            state.reg[destReg] = state.reg[regA] + state.reg[regB];
        }
        else if (op == 1) {  // nor
            state.reg[destReg] = ~(state.reg[regA] | state.reg[regB]);
        }
        else if (op == 2) {  // lw
            state.reg[regB] = state.mem[state.reg[regA] + offset];
        }
        else if (op == 3) {  // sw
            state.mem[state.reg[regA] + offset] = state.reg[regB];
        }
        else if (op == 4) {  // beq
            if (state.reg[regA] == state.reg[regB]) {
                state.pc += offset;
            }
        }
        else if (op == 5) {  // jalr
            state.reg[regB] = state.pc + 1;
            state.pc = state.reg[regA] - 1;
        }
        else if (op == 6) {  // halt
            halt = 1;
        }
        else if (op == 7) {  // noop
            continue;
        }

    }

    printf("machine halted\ntotal of %d instructions executed\n", numInstruct);
    printf("final state of machine:\n");
    printState(&state);

    return(0);
}

void
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
              printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
              printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int
convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1<<15) ) {
        num -= (1<<16);
    }
    return(num);
}

int
getRegA(int mc)
{
    uint32_t regA = mc << 10;
    regA = regA >> 29;
    return regA;
}

int
getRegB(int mc)
{
    uint16_t temp16 = mc >> 3;
    return temp16 >> 13;
}

int
getRegDest(uint8_t mc)
{
    return mc;
}

int
getOffset(int mc)
{
    uint16_t MSB = mc, offset = mc;
    MSB = MSB >> 15;
    offset = offset << 1;
    offset = offset >> 1;
    return ((MSB * -32768) + offset);
}