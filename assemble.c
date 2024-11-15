#include <stdio.h>
#include "symtab.h"

// Declare the pass functions
void pass1(const char* inputFile, const char* intermediateFile);
void pass2(char* intermediate);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./assemble <input_file>\n");
        return 1;
    }

    // Run both passes
    printf("Starting Pass 1...\n");
    pass1(argv[1], "intermediate.txt");
    
    printf("\nStarting Pass 2...\n");
    pass2("intermediate.txt");
    
    printf("\nAssembly complete!\n");
    return 0;
} 