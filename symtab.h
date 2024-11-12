#ifndef SYMTAB_H
#define SYMTAB_H

#define MAX_SYMBOLS 100
#define MAX_LABEL_LENGTH 20

typedef struct SymTab 
{
    char label[MAX_LABEL_LENGTH];
    unsigned int address;
} SYMBOL;

int loadSymbolTable(const char* symTabFile, SYMBOL* symtab, int* symCount);
int getSymbolAddress(const char* label, SYMBOL* symtab, int* symCount);

#endif 