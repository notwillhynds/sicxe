#include <stdio.h>
#include <string.h>
#include "tables.h"

#define OPTAB_LENGTH 59  // Updated length for complete SIC/XE instruction set
#define REGTAB_LENGTH 9

//Operand Table For SIC/XE
OPCODE OPTAB[] = {
    // Format 3/4 Instructions
    {"ADD",   3, 0x18},
    {"ADDF",  3, 0x58},
    {"AND",   3, 0x40},
    {"COMP",  3, 0x28},
    {"COMPF", 3, 0x88},
    {"DIV",   3, 0x24},
    {"DIVF",  3, 0x64},
    {"J",     3, 0x3C},
    {"JEQ",   3, 0x30},
    {"JGT",   3, 0x34},
    {"JLT",   3, 0x38},
    {"JSUB",  3, 0x48},
    {"LDA",   3, 0x00},
    {"LDB",   3, 0x68},
    {"LDCH",  3, 0x50},
    {"LDF",   3, 0x70},
    {"LDL",   3, 0x08},
    {"LDS",   3, 0x6C},
    {"LDT",   3, 0x74},
    {"LDX",   3, 0x04},
    {"LPS",   3, 0xD0},
    {"MUL",   3, 0x20},
    {"MULF",  3, 0x60},
    {"OR",    3, 0x44},
    {"RD",    3, 0xD8},
    {"RSUB",  3, 0x4C},
    {"SSK",   3, 0xEC},
    {"STA",   3, 0x0C},
    {"STB",   3, 0x78},
    {"STCH",  3, 0x54},
    {"STF",   3, 0x80},
    {"STI",   3, 0xD4},
    {"STL",   3, 0x14},
    {"STS",   3, 0x7C},
    {"STSW",  3, 0xE8},
    {"STT",   3, 0x84},
    {"STX",   3, 0x10},
    {"SUB",   3, 0x1C},
    {"SUBF",  3, 0x5C},
    {"TD",    3, 0xE0},
    {"TIX",   3, 0x2C},
    {"WD",    3, 0xDC},

    // Format 2 Instructions
    {"ADDR",  2, 0x90},
    {"CLEAR", 2, 0xB4},
    {"COMPR", 2, 0xA0},
    {"DIVR",  2, 0x9C},
    {"MULR",  2, 0x98},
    {"RMO",   2, 0xAC},
    {"SHIFTL",2, 0xA4},
    {"SHIFTR",2, 0xA8},
    {"SUBR",  2, 0x94},
    {"SVC",   2, 0xB0},
    {"TIXR",  2, 0xB8},

    // Format 1 Instructions
    {"FIX",   1, 0xC4},
    {"FLOAT", 1, 0xC0},
    {"HIO",   1, 0xF4},
    {"NORM",  1, 0xC8},
    {"SIO",   1, 0xF0},
    {"TIO",   1, 0xF8}
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

//Register Table
REGISTER REGTAB[] = {
    {"A", 0},
    {"X", 1},
    {"L", 2},
    {"B", 3},
    {"S", 4},
    {"T", 5},
    {"F", 6},
    {"PC", 8},
    {"SW", 9}

};

//Retrive Register Value
int getRegister(const char* mnemonic) {
    for(int i = 0; i < REGTAB_LENGTH; i++) {
        if(strcmp(mnemonic, REGTAB[i].mnemonic) == 0) {
            return REGTAB[i].num;
        }
    }
    return -1;
}