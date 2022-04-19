/*
 * Project 3
 * EECS 370 LC-2K Pipelined simulator
 */

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR will not implemented for Project 3 */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDStruct {
    int instr;
    int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
    int instr;
    int pcPlus1;
    int readRegA;
    int readRegB;
    int offset;
} IDEXType;

typedef struct EXMEMStruct {
    int instr;
    int branchTarget;
    int aluResult;
    int readRegB;
} EXMEMType;

typedef struct MEMWBStruct {
    int instr;
    int writeData;
} MEMWBType;

typedef struct WBENDStruct {
    int instr;
    int writeData;
} WBENDType;

typedef struct stateStruct {
    int pc;
    int instrMem[NUMMEMORY];
    int dataMem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
    IFIDType IFID;
    IDEXType IDEX;
    EXMEMType EXMEM;
    MEMWBType MEMWB;
    WBENDType WBEND;
    int cycles; /* number of cycles run so far */
} stateType;

void printState(stateType *);
int field0(int instruction);
int field1(int instruction);
int field2(int instruction);
int opcode(int instruction);
void printInstruction(int instr);
//int convertNum(int);
//int getRegA(int);
//int getRegB(int);
//int getRegDest(uint8_t);
//int getOffset(int);

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

        if (sscanf(line, "%d", state.dataMem + state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.dataMem[state.numMemory]);

        state.instrMem[state.numMemory] = state.dataMem[state.numMemory];
    }

    printf("%d memory words\n", state.numMemory);

    printf("\tinstruction memory:\n");

    for (int i = 0; i < state.numMemory; ++i) {
        printf("\t\tinstrMem[ %d ] ", i);
        printInstruction(state.instrMem[i]);
    }



    // Set all reg to 0
    for (int i = 0; i < NUMREGS; ++i) {
        state.reg[i] = 0;
    }

    // Set rest of mem to 0
    for (int i = state.numMemory; i < NUMMEMORY; ++i) {
        state.instrMem[i] = 0;
        state.dataMem[i] = 0;
    }

    // Set pc to 0
    state.pc = 0;
    state.cycles = 0;

    // Set all instr to noop
    state.IFID.instr = NOOPINSTRUCTION;
    state.IDEX.instr = NOOPINSTRUCTION;
    state.EXMEM.instr = NOOPINSTRUCTION;
    state.MEMWB.instr = NOOPINSTRUCTION;
    state.WBEND.instr = NOOPINSTRUCTION;

    printf("\n");

    stateType newState;

    while (1) {

        printState(&state);

        /* check for halt */
        if (opcode(state.MEMWB.instr) == HALT) {
            printf("machine halted\n");
            printf("total of %d cycles executed\n", state.cycles);
            exit(0);
        }

        newState = state;
        newState.cycles++;      

        /* --------------------- IF stage --------------------- */
        newState.IFID.instr = state.instrMem[state.pc];

        newState.IFID.pcPlus1 = state.pc + 1;
        newState.pc++;

        /* --------------------- ID stage --------------------- */
        if ((opcode(state.IDEX.instr) == LW                                                                                         // prev = LW
            && (opcode(state.IFID.instr) < JALR))                                                                                   // curr = ADD, NOR, LW, SW, BEQ
            && (field0(state.IFID.instr) == field1(state.IDEX.instr) || field1(state.IFID.instr) == field1(state.IDEX.instr))) {    // prev regB == regA || regB
            newState.pc = state.pc;
            newState.IFID.instr = state.IFID.instr;
            newState.IFID.pcPlus1 = state.IFID.pcPlus1;
            newState.IDEX.instr = NOOPINSTRUCTION;              // Insert stall for LW if data depenency
        }
        else {
            newState.IDEX.instr = state.IFID.instr;
            newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
            newState.IDEX.readRegA = state.reg[field0(newState.IDEX.instr)];
            newState.IDEX.readRegB = state.reg[field1(newState.IDEX.instr)];
            newState.IDEX.offset = convertNum(field2(newState.IDEX.instr));
        }

        /* --------------------- EX stage --------------------- */
        newState.EXMEM.instr = state.IDEX.instr;
        newState.EXMEM.branchTarget = state.IDEX.pcPlus1 + state.IDEX.offset;

        //int regA = state.IDEX.readRegA, regB = state.IDEX.readRegB;
        int regA = state.reg[field0(state.IDEX.instr)], regB = state.reg[field1(state.IDEX.instr)];

        //  LW 3 prev
        if (opcode(state.WBEND.instr) == LW) {
            if (field1(state.WBEND.instr) == field0(newState.EXMEM.instr)) {      // prev regB == regA
                regA = state.WBEND.writeData;
            }

            if (field1(state.WBEND.instr) == field1(newState.EXMEM.instr)) {      // prev regB == regB
                regB = state.WBEND.writeData;
            }
        }

        //  LW 2 prev
        if (opcode(state.MEMWB.instr) == LW) {
            if (field1(state.MEMWB.instr) == field0(newState.EXMEM.instr)) {      // prev regB == regA
                regA = state.MEMWB.writeData;
            }

            if (field1(state.MEMWB.instr) == field1(newState.EXMEM.instr)) {      // prev regB == regB
                regB = state.MEMWB.writeData;
            }
        }

        // ADD/NOR 2 prev
        if (opcode(state.MEMWB.instr) <= NOR) {
            if (field2(state.MEMWB.instr) == field0(newState.EXMEM.instr)) {        // prev destReg == regA
                regA = state.MEMWB.writeData;
            }

            if (field2(state.MEMWB.instr) == field1(newState.EXMEM.instr)) {    // prev destReg == regB
                regB = state.MEMWB.writeData;
            }
        }
        
        //  ADD/NOR 1 prev
        if (opcode(state.EXMEM.instr) <= NOR) {
            if (field2(state.EXMEM.instr) == field0(newState.EXMEM.instr)) {        // prev destReg == regA
                regA = state.EXMEM.aluResult;
            }

            if (field2(state.EXMEM.instr) == field1(newState.EXMEM.instr)) {    // prev destReg == regB
                regB = state.EXMEM.aluResult;
            }
        }

        // ALU result
        if (opcode(newState.EXMEM.instr) == ADD) {
            newState.EXMEM.aluResult = regA + regB;        // regA + regB
        }
        else if (opcode(newState.EXMEM.instr) == NOR) {
            newState.EXMEM.aluResult = ~(regA | regB);     // regA NOR regB
        }
        else if (opcode(newState.EXMEM.instr) == LW) {
            newState.EXMEM.aluResult = regA + state.IDEX.offset;          // regA + offset
        }
        else if (opcode(newState.EXMEM.instr) == SW) {
            newState.EXMEM.aluResult = regA + state.IDEX.offset;          // regA + offset
        }
        else if (opcode(newState.EXMEM.instr) == BEQ) {

            if (regA == regB)
                newState.EXMEM.aluResult = 1;        // regA == regB
            else
                newState.EXMEM.aluResult = 0;        // regA != regB
        }

        newState.EXMEM.readRegB = regB;

        /* --------------------- MEM stage --------------------- */
        newState.MEMWB.instr = state.EXMEM.instr;

        
        if (opcode(newState.MEMWB.instr) == ADD || opcode(newState.MEMWB.instr) == NOR) {
            newState.MEMWB.writeData = state.EXMEM.aluResult;                   // ALU result
        }
        else if (opcode(newState.MEMWB.instr) == LW) {
            newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];   // mem[ALU result]
        }
        else if (opcode(newState.MEMWB.instr) == SW) {
            newState.MEMWB.writeData = state.EXMEM.readRegB;                    // regB
        }
        else if (opcode(newState.MEMWB.instr) == BEQ) {
            if (state.EXMEM.aluResult) {
                newState.pc = state.EXMEM.branchTarget;
                newState.IFID.instr = NOOPINSTRUCTION;          // Update pc and discard
                newState.IDEX.instr = NOOPINSTRUCTION;          // instr when not taken
                newState.EXMEM.instr = NOOPINSTRUCTION;
            }
        }
        
        /* --------------------- WB stage --------------------- */
        newState.WBEND.instr = state.MEMWB.instr;

        newState.WBEND.writeData = state.MEMWB.writeData;

        // Writing back
        if (opcode(newState.WBEND.instr) == ADD || opcode(newState.WBEND.instr) == NOR) {
            newState.reg[field2(newState.WBEND.instr)] = newState.WBEND.writeData;      // destReg = ALU result
        }
        else if (opcode(newState.WBEND.instr) == LW) {
            newState.reg[field1(newState.WBEND.instr)] = newState.WBEND.writeData;      // regB = mem[regA + offset]
        }
        else if (opcode(newState.WBEND.instr) == SW) {
            newState.dataMem[newState.reg[field0(newState.WBEND.instr)] + field2(newState.WBEND.instr)] = newState.WBEND.writeData; // mem[regA + offset] = regB
        }

        state = newState; /* this is the last statement before end of the loop.
                It marks the end of the cycle and updates the
                current state with the values calculated in this
                cycle */
    }
    
    return(0);
}

void
printState(stateType* statePtr)
{
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (i = 0; i < statePtr->numMemory; i++) {
        printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("\tIFID:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IFID.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
    printf("\tIDEX:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IDEX.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
    printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
    printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
    printf("\t\toffset %d\n", statePtr->IDEX.offset);
    printf("\tEXMEM:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->EXMEM.instr);
    printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
    printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
    printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);
    printf("\tMEMWB:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->MEMWB.instr);
    printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);
    printf("\tWBEND:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->WBEND.instr);
    printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
}

int
field0(int instruction)
{
    return((instruction >> 19) & 0x7);
}

int
field1(int instruction)
{
    return((instruction >> 16) & 0x7);
}

int
field2(int instruction)
{
    return(instruction & 0xFFFF);
}

int
opcode(int instruction)
{
    return(instruction >> 22);
}

void
printInstruction(int instr)
{

    char opcodeString[10];

    if (opcode(instr) == ADD) {
        strcpy(opcodeString, "add");
    }
    else if (opcode(instr) == NOR) {
        strcpy(opcodeString, "nor");
    }
    else if (opcode(instr) == LW) {
        strcpy(opcodeString, "lw");
    }
    else if (opcode(instr) == SW) {
        strcpy(opcodeString, "sw");
    }
    else if (opcode(instr) == BEQ) {
        strcpy(opcodeString, "beq");
    }
    else if (opcode(instr) == JALR) {
        strcpy(opcodeString, "jalr");
    }
    else if (opcode(instr) == HALT) {
        strcpy(opcodeString, "halt");
    }
    else if (opcode(instr) == NOOP) {
        strcpy(opcodeString, "noop");
    }
    else {
        strcpy(opcodeString, "data");
    }
    printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
        field2(instr));
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
//
//int
//getRegA(int mc)
//{
//    uint32_t regA = mc << 10;
//    regA = regA >> 29;
//    return regA;
//}
//
//int
//getRegB(int mc)
//{
//    uint16_t temp16 = mc >> 3;
//    return temp16 >> 13;
//}
//
//int
//getRegDest(uint8_t mc)
//{
//    return mc;
//}
//
//int
//getOffset(int mc)
//{
//    uint16_t MSB = mc, offset = mc;
//    MSB = MSB >> 15;
//    offset = offset << 1;
//    offset = offset >> 1;
//    return ((MSB * -32768) + offset);
//}