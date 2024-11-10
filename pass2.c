#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables.h"
#include "symtab.h"

#define MAX_LINE_LENGTH 1000

void tokenize(char* intermediate) {
    FILE* file = fopen(intermediate, "r");
    if(file == NULL) {
        printf("Error opening file\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    char opcode[50], operand[50];

    while(fgets(line, sizeof(line), file)){
        printf("%s", line);
    }

}

int main() {
    SYMBOL symtab[MAX_SYMBOLS];
    int symCount = 0;

    loadSymbolTable("symtab.txt", symtab, &symCount);
        // printf("Loaded symbol: %s at address: %04X\n", 
        //     symtab[1].label, 
        //     symtab[1].address);
    tokenize("intermediate.txt");
    }
