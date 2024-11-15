#ifndef SYMTAB_H
#define SYMTAB_H

#define MAX_SYMBOLS 1000
#define MAX_LABEL_LENGTH 20

// Symbol table structure
typedef struct {
    char label[MAX_LABEL_LENGTH];
    unsigned int address;
} SYMBOL;

// Global symbol table and count
extern SYMBOL SYMTAB[MAX_SYMBOLS];
extern int symCount;

// Function declarations
int searchSymTab(const char* label);
int addSymbol(const char* label, unsigned int address);
int loadSymbolTable(const char* symTabFile, SYMBOL* symtab, int* symCount);
int getSymbolAddress(const char* label, SYMBOL* symtab, int* symCount);

#endif