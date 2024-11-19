#include <stdio.h>
#include <string.h>
#include "tables.h"

#define OPTAB_LENGTH 41
#define REG_TABLE_LENGTH 9

//Operand Table For SIC/XE
OPCODE OPTAB[] = {
    {"ADD",  0x18, 3},
    {"ADDF", 0x58, 3},
    {"AND",  0x40, 3},
    {"COMP", 0x28, 3},
    {"COMPF",0x88, 3},
    {"DIV",  0x24, 3},
    {"DIVF", 0x64, 3},
    {"J",    0x3C, 3},
    {"JEQ",  0x30, 3},
    {"JGT",  0x34, 3},
    {"JLT",  0x38, 3},
    {"JSUB", 0x48, 3},
    {"LDA",  0x00, 3},
    {"LDB",  0x68, 3},
    {"LDCH", 0x50, 3},
    {"LDF",  0x70, 3},
    {"LDL",  0x08, 3},
    {"LDS",  0x6C, 3},
    {"LDT",  0x74, 3},
    {"LDX",  0x04, 3},
    {"LPS",  0xD0, 3},
    {"MUL",  0x20, 3},
    {"MULF", 0x60, 3},
    {"OR",   0x44, 3},
    {"RD",   0xD8, 3},
    {"RSUB", 0x4C, 3},
    {"STA",  0x0C, 3},
    {"STB",  0x78, 3},
    {"STCH", 0x54, 3},
    {"STF",  0x80, 3},
    {"STI",  0xD4, 3},
    {"STL",  0x14, 3},
    {"STS",  0x7C, 3},
    {"STSW", 0xE8, 3},
    {"STT",  0x84, 3},
    {"STX",  0x10, 3},
    {"SUB",  0x1C, 3},
    {"SUBF", 0x5C, 3},
    {"TD",   0xE0, 3},
    {"TIX",  0x2C, 3},
    {"WD",   0xDC, 3},
    
    {"CLEAR", 0xB4, 2},
    {"COMPR", 0xA0, 2},
    {"TIXR",  0xB8, 2},
    {"ADDR",  0x90, 2},
    {"DIVR",  0x9C, 2},
    {"MULR",  0x98, 2},
    {"RMO",   0xAC, 2},
    {"SHIFTL",0xA4, 2},
    {"SHIFTR",0xA8, 2},
    {"SUBR",  0x94, 2},
    
    {"FIX",   0xC4, 1},
    {"FLOAT", 0xC0, 1},
    {"HIO",   0xF4, 1},
    {"NORM",  0xC8, 1},
    {"SIO",   0xF0, 1},
    {"TIO",   0xF8, 1}
};

// Add this register table
REG REGISTERS[] = {
    {"A",  0},  // Accumulator
    {"X",  1},  // Index Register
    {"L",  2},  // Linkage Register
    {"B",  3},  // Base Register
    {"S",  4},  // General Purpose Register
    {"T",  5},  // General Purpose Register
    {"F",  6},  // Floating Point Register
    {"PC", 8},  // Program Counter
    {"SW", 9}   // Status Word
};

unsigned int getOpcode(const char* mnemonic) {
    for(int i = 0; i < OPTAB_LENGTH; i++) {
        if(strcmp(OPTAB[i].mnemonic, mnemonic) == 0) {
            return OPTAB[i].opcode;
        }
    }
    printf("Error: Opcode not found for %s\n", mnemonic);
    return 0xFF;  // Return invalid opcode if not found
}

//Search Functionality
int searchOpTab(const char* opcode) {
    for(int i = 0; i < OPTAB_LENGTH; i++) {
        if(strcmp(opcode, OPTAB[i].mnemonic) == 0) {
            return OPTAB[i].format;
        }
    }
    return -1;
}

// Add this search function
int getRegisterNum(const char* regName) {
    for(int i = 0; i < REG_TABLE_LENGTH; i++) {
        if(strcmp(regName, REGISTERS[i].mnemonic) == 0) {
            return REGISTERS[i].num;
        }
    }
    printf("Error: Register %s not found\n", regName);
    return -1;  // Return -1 if register not found
}