#include <stdio.h>
#include <string.h>
#include "symtab.h"



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
int getSymbolAddress(const char* label, SYMBOL* symtab, int* symCount) {
    for(int i = 0; i < *symCount; i++) {
        if(strcmp(symtab[i].label, label) == 0) {
            return symtab[i].address;
        }
    }
    return 0xFFFF;
}



