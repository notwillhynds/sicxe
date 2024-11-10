#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabels.h"
#include "symtab.h"

int main() {
    SYMBOL symtab[MAX_SYMBOLS];
    int symCount = 0;

    loadSymbolTable("symtab.txt", symtab, &symCount);
        printf("Loaded symbol: %s at address: %04X\n", 
            symtab[1].label, 
            symtab[1].address);

}
