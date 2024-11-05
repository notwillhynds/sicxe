#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100
#define MAX_LABEL_LENGTH 20
#define MAX_OPCODE_LENGTH 10
#define MAX_OPERAND_LENGTH 20
#define MAX_SYMBOLS 100
#define MAX_OPCODES 50

// Symbol table structure
struct Symbol {
    char label[MAX_LABEL_LENGTH];
    int address;
};

// Opcode table structure
struct OpCode {
    char mnemonic[7];
    int format;
};

// Global tables
struct Symbol SYMTAB[MAX_SYMBOLS];
int symCount = 0;

// Predefined OPTAB
struct OpCode OPTAB[] = {
    {"LDA", 3}, {"STL", 3}, {"JSUB", 3}, {"COMP", 3},
    {"JEQ", 3}, {"J", 3}, {"STA", 3}, {"LDL", 3},
    {"RSUB", 3}, {"LDX", 3}, {"TD", 3}, {"RD", 3},
    {"STCH", 3}, {"TIX", 3}, {"JLT", 3}, {"WD", 3},
    {"LDCH", 3}
};

// Function to search OPTAB
int searchOptab(const char *opcode) {
    int numOpcodes = sizeof(OPTAB) / sizeof(OPTAB[0]);
    for(int i = 0; i < numOpcodes; i++) {
        if(strcmp(OPTAB[i].mnemonic, opcode) == 0) {
            return OPTAB[i].format;
        }
    }
    return -1;  // Not found
}

// Function to search SYMTAB
int searchSymtab(const char *label) {
    for(int i = 0; i < symCount; i++) {
        if(strcmp(SYMTAB[i].label, label) == 0) {
            return i;  // Found - return index
        }
    }
    return -1;  // Not found
}

// Function to add symbol to SYMTAB
int addSymbol(const char *label, int address) {
    // Check for duplicate
    if(searchSymtab(label) != -1) {
        printf("Error: Duplicate symbol '%s'\n", label);
        return 0;  // Error
    }

    // Add new symbol
    if(symCount < MAX_SYMBOLS) {
        strcpy(SYMTAB[symCount].label, label);
        SYMTAB[symCount].address = address;
        symCount++;
        printf("Added symbol: %s at address: %04X\n", label, address);
        return 1;  // Success
    }
    return 0;  // Error
}

void pass1(const char *inputFile, const char *intermediateFile) {
    FILE *input = fopen(inputFile, "r");
    FILE *intermediate = fopen(intermediateFile, "w");
    
    if (input == NULL || intermediate == NULL) {
        printf("Error: Cannot open files\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    char label[MAX_LABEL_LENGTH] = "";
    char opcode[MAX_OPCODE_LENGTH] = "";
    char operand[MAX_OPERAND_LENGTH] = "";
    int LOCCTR = 0;
    int startingAddr = 0;
    int lineNumber = 5;
    int errorFlag = 0;

    // Write header to intermediate file
    fprintf(intermediate, "%-6s\t%-8s\t%-8s\t%-8s\t%-8s\t%s\n",
            "Line", "Address", "Label", "OPCODE", "OPERAND", "Comment");
    fprintf(intermediate, "------------------------------------------------------------------------------\n");

    // Process first line (START)
    if (fgets(line, MAX_LINE_LENGTH, input)) {
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "%s %s %s", label, opcode, operand);

        if (strcmp(opcode, "START") == 0) {
            LOCCTR = (int)strtol(operand, NULL, 16);
            startingAddr = LOCCTR;
            if (strlen(label) > 0) {
                addSymbol(label, LOCCTR);
            }
            fprintf(intermediate, "%-6d\t%04X\t%-8s\t%-8s\t%-8s\n", 
                    lineNumber, LOCCTR, label, opcode, operand);
        } else {
            LOCCTR = 0;
            startingAddr = 0;
        }

        // Main processing loop
        while (fgets(line, MAX_LINE_LENGTH, input)) {
            lineNumber += 5;
            line[strcspn(line, "\n")] = 0;
            
            // Clear previous values
            label[0] = opcode[0] = operand[0] = '\0';
            
            // Parse line
            sscanf(line, "%s %s %s", label, opcode, operand);

            // Check for comment line
            if (line[0] == '.') {
                fprintf(intermediate, "%-6d\t    \t        \t        \t        \t%s\n", 
                        lineNumber, line);
                continue;
            }

            // Process non-comment line
            if (line[0] != '.') {
                // Process label if present
                if (strlen(label) > 0 && line[0] != ' ') {
                    if (!addSymbol(label, LOCCTR)) {
                        errorFlag = 1;
                    }
                }

                // Process opcode
                if (strcmp(opcode, "END") != 0) {
                    int format = searchOptab(opcode);
                    if (format > 0) {
                        LOCCTR += format;
                    }
                    else if (strcmp(opcode, "WORD") == 0) {
                        LOCCTR += 3;
                    }
                    else if (strcmp(opcode, "RESW") == 0) {
                        LOCCTR += 3 * atoi(operand);
                    }
                    else if (strcmp(opcode, "RESB") == 0) {
                        LOCCTR += atoi(operand);
                    }
                    else if (strcmp(opcode, "BYTE") == 0) {
                        if (operand[0] == 'C') {
                            LOCCTR += strlen(operand) - 3;  // Subtract 3 for C''
                        }
                        else if (operand[0] == 'X') {
                            LOCCTR += (strlen(operand) - 3) / 2;  // Subtract 3 for X''
                        }
                    }
                    else {
                        printf("Error: Invalid opcode '%s' at line %d\n", opcode, lineNumber);
                        errorFlag = 1;
                    }
                }
            }

            // Write to intermediate file
            fprintf(intermediate, "%-6d\t%04X\t%-8s\t%-8s\t%-8s\n", 
                    lineNumber, LOCCTR, label, opcode, operand);

            // Check for END
            if (strcmp(opcode, "END") == 0) {
                break;
            }
        }
    }

    // Print results
    printf("\nProgram length: %04X\n", LOCCTR - startingAddr);
    if (errorFlag) {
        printf("Errors found during assembly\n");
    }

    // Print symbol table
    printf("\nSymbol Table:\n");
    printf("Label\tAddress\n");
    printf("---------------\n");
    for (int i = 0; i < symCount; i++) {
        printf("%-6s\t%04X\n", SYMTAB[i].label, SYMTAB[i].address);
    }

    fclose(input);
    fclose(intermediate);
}
