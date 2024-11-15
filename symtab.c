#include <stdio.h>
#include <string.h>
#include "symtab.h"

// Global variables
SYMBOL SYMTAB[MAX_SYMBOLS];
int symCount = 0;

// Search symbol table
int searchSymTab(const char* label) {
    for(int i = 0; i < symCount; i++) {
        if(strcmp(SYMTAB[i].label, label) == 0) {
            return i;
        }
    }
    return -1;
}

// Add symbol to table
int addSymbol(const char* label, unsigned int address) {
    if (searchSymTab(label) != -1) {
        printf("Error: Duplicate Symbol Found '%s'\n", label);
        return 0;
    }
    else if(symCount < MAX_SYMBOLS && strlen(label) > 0) {
        strcpy(SYMTAB[symCount].label, label);
        SYMTAB[symCount].address = address;
        symCount++;
        printf("Added symbol: %s at address: %04X\n", label, address);
        return 1;
    }
    return 0;
}

// Load symbol table from file
int loadSymbolTable(const char* symTabFile, SYMBOL* symtab, int* symcount) {
    FILE* file = fopen(symTabFile, "r");
    if(!file) return 0;
    
    char line[100];
    *symcount = 0;
    
    while(fgets(line, sizeof(line), file) && *symcount < MAX_SYMBOLS) {
        char label[MAX_LABEL_LENGTH];
        int addr;
        sscanf(line, "%s %x", label, &addr);
        strcpy(symtab[*symcount].label, label);
        symtab[*symcount].address = addr;
        (*symcount)++;
    }
    fclose(file);
    return 1;
}

// Get symbol address
int getSymbolAddress(const char* label, SYMBOL* symtab, int* symCount) {
    for(int i = 0; i < *symCount; i++) {
        if(strcmp(symtab[i].label, label) == 0) {
            return symtab[i].address;
        }
    }
    return 0xFFFF;
}



