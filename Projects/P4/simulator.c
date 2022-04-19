#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
//#include <cmath>

#define MAX_CACHE_SIZE 256
#define MAX_BLOCK_SIZE 256

enum actionType
{
    cacheToProcessor,
    processorToCache,
    memoryToCache,
    cacheToMemory,
    cacheToNowhere
};

typedef struct blockStruct
{
    int data[MAX_BLOCK_SIZE];
    bool isDirty;
    int lruLabel;
    int set;
    int tag;
} blockStruct;

typedef struct cacheStruct
{
    blockStruct blocks[MAX_CACHE_SIZE];
    int blocksPerSet;
    int blockSize;
    int lru;
    int numSets;
} cacheStruct;

/* Global Cache variable */
cacheStruct cache;

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

/*
 * Log the specifics of each cache action.
 *
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *  -	cacheToProcessor: reading data from the cache to the processor
 *  -	processorToCache: writing data from the processor to the cache
 *  -	memoryToCache: reading data from the memory to the cache
 *  -	cacheToMemory: evicting cache data and writing it to the memory
 *  -	cacheToNowhere: evicting cache data and throwing it away
 */

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

int load(int addr);
void store(int addr, int data);
void updateLRU(int set, int block);
void printAction(int address, int size, enum actionType type);
void getBits(int addr, uint32_t* blockOffset, uint32_t* setIndex, uint32_t* tag);
int getAddr(blockStruct block, int i);
int getRegA(int);
int getRegB(int);
int getRegDest(uint8_t);
int getOffset(int);

stateType state;

int
main(int argc, char* argv[])
{
    char line[MAXLINELENGTH];
    FILE* filePtr;

    if (argc != 5) {
        printf("error: usage: %s <machine-code file> <blockSizeInWords> <numberOfSets> <blocksPerSet>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    cache.blockSize = atoi(argv[2]);
    cache.numSets = atoi(argv[3]);
    cache.blocksPerSet = atoi(argv[4]);

    // Set all tags to -1, set highest LRU to first block, Assign valid sets
    for (int i = 0, LRU = cache.blocksPerSet - 1, set = 0; i < (cache.blocksPerSet * cache.numSets); i++) {
        cache.blocks[i].tag = -1;
        cache.blocks[i].lruLabel = LRU;
        cache.blocks[i].set = set;
        cache.blocks[i].isDirty = false;

        if (i + 1 == ((set + 1) * cache.blocksPerSet)) {
            ++set;
            LRU = cache.blocksPerSet - 1;
        }
        else
            --LRU;
    }

    /* read the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
        state.numMemory++) {

        if (sscanf(line, "%d", state.mem + state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }

    }

    int halt = 0;

    for (int i = 0; i < NUMREGS; ++i) {
        state.reg[i] = 0;
    }

    /* Processor */
    for (state.pc = 0; !halt; ++state.pc) {
        int mc = load(state.pc);
        printAction(state.pc, 1, cacheToProcessor);
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
            state.reg[regB] = load(state.reg[regA] + offset);
            printAction(state.reg[regA] + offset, 1, cacheToProcessor);
        }
        else if (op == 3) {  // sw
            store(state.reg[regA] + offset, state.reg[regB]);
            printAction(state.reg[regA] + offset, 1, processorToCache);

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

    return(0);
}

// Properly simulates the cache for a load from
// memory address “addr”. Returns the loaded value.
int load(int addr)
{
    uint32_t tag, set, block;
    getBits(addr, &tag, &set, &block);
    int index = 0;
    int LRU = -1;

    // Search set
    for (int i = set * cache.blocksPerSet; i < ((set + 1) * cache.blocksPerSet); i++) {
        if (cache.blocks[i].tag == tag) {           // If tag is in cache set 
            updateLRU(set, i);
            return cache.blocks[i].data[block];     // no load needed, return
        }
        else if (cache.blocks[i].set == set && cache.blocks[i].lruLabel > LRU) {    // else find LRU
            index = i;                              // keep track of cache block to evict
            LRU = cache.blocks[i].lruLabel;         // keep track of LRU
        }
    }

    // If cache block is dirty, store into memory
    if (cache.blocks[index].isDirty) {
        for (int i = 0; i < cache.blockSize; i++) {
            state.mem[getAddr(cache.blocks[index], i)] = cache.blocks[index].data[i];
        }

        printAction(getAddr(cache.blocks[index], 0), cache.blockSize, cacheToMemory);
    }
    else if (cache.blocks[index].tag != -1) {
        printAction(getAddr(cache.blocks[index], 0), cache.blockSize, cacheToNowhere);
    }

    // Load cache block
    cache.blocks[index].tag = tag;
    //cache.blocks[index].set = set;
    cache.blocks[index].isDirty = false;
    int offset = (addr / cache.blockSize) * cache.blockSize;
    for (int i = 0; i < cache.blockSize; i++) {
        cache.blocks[index].data[i] = state.mem[offset + i];
    }

    printAction(offset, cache.blockSize, memoryToCache);

    updateLRU(set, index);

    return cache.blocks[index].data[block];
}

// Properly simulates the cache for a store
// to memory address “addr”. Returns nothing.
void store(int addr, int data)
{
    // Load if necessary
    load(addr);

    uint32_t block, set, tag;
    getBits(addr, &tag, &set, &block);
    int index = 0;

    for (int i = set * cache.blocksPerSet; i < ((set + 1) * cache.blocksPerSet); i++) {
        if (cache.blocks[i].tag == tag) {           // If tag is in cache set
            index = i;
            break;
        }
    }

    // Store data into corresponding block and set dirty bit
    cache.blocks[index].data[block] = data;
    cache.blocks[index].isDirty = true;
}

void updateLRU(int set, int block) {
    // Update LRUs in set
    if (cache.blocks[block].lruLabel != 0) {
        for (int i = set * cache.blocksPerSet; i < ((set + 1) * cache.blocksPerSet); i++) {

            if (cache.blocks[i].lruLabel < cache.blocks[block].lruLabel) {
                ++cache.blocks[i].lruLabel;
                //cache.blocks[i].lruLabel %= cache.blocksPerSet;
            }
        }

        cache.blocks[block].lruLabel = 0;
    }
}

void getBits(int addr, uint32_t* tag, uint32_t* setIndex, uint32_t* blockOffset)
{

    int blockBits = (int)(log(cache.blockSize) / log(2));
    *blockOffset = addr << (32 - blockBits);
    *blockOffset = *blockOffset >> (32 - blockBits);
    if (cache.blockSize == 1)
        *blockOffset = 0;

    int setBits = (int)(log(cache.numSets) / log(2));
    *setIndex = addr >> blockBits;

    uint32_t temp = 0;
    for (int i = 0; i < setBits; ++i) {
        temp = temp << 1;
        temp += 1;
    }

    *setIndex &= temp;

    *tag = addr >> (blockBits + setBits);
}

int getAddr(blockStruct block, int i)
{
    int addr = 0;
    addr = (block.set * cache.blockSize) + i;
    int tagOffset = (log(cache.blockSize) / log(2)) + (log(cache.numSets) / log(2));
    addr += (block.tag << tagOffset);
    return addr;
}


void printAction(int address, int size, enum actionType type)
{
    printf("@@@ transferring word [%d-%d] ", address, address + size - 1);

    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    }
    else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    }
    else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    }
    else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    }
    else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
}

/*
 * Prints the cache based on the configurations of the struct
 */
void printCache()
{
    printf("\n@@@\ncache:\n");

    for (int set = 0; set < cache.numSets; ++set) {
        printf("\tset %i:\n", set);
        for (int block = 0; block < cache.blocksPerSet; ++block) {
            printf("\t\t[ %i ]: {", block);
            for (int index = 0; index < cache.blockSize; ++index) {
                printf(" %i", cache.blocks[set * cache.blocksPerSet + block].data[index]);
            }
            printf(" }\n");
        }
    }

    printf("end cache\n");
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