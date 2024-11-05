#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100
#define MAX_LABEL_LENGTH 20
#define MAX_OPCODE_LENGTH 10
#define MAX_OPERAND_LENGTH 20
#define MAX_SYMBOLS 100

// Symbol table structure
struct Symbol {
    char label[MAX_LABEL_LENGTH];
    int address;
};

// Global symbol table
struct Symbol SYMTAB[MAX_SYMBOLS];
int symCount = 0;

// Function to add symbol to SYMTAB
void addSymbol(const char *label, int address) {
    if (symCount < MAX_SYMBOLS && strlen(label) > 0) {
        strcpy(SYMTAB[symCount].label, label);
        SYMTAB[symCount].address = address;
        symCount++;
        printf("Added symbol: %s at address: %04X\n", label, address);
    }
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

    // Write header to intermediate file
    fprintf(intermediate, "%-6s\t%-8s\t%-8s\t%-8s\t%-8s\t%s\n",
            "Line", "Address", "Label", "OPCODE", "OPERAND", "Comment");
    fprintf(intermediate, "------------------------------------------------------------------------------\n");

    // Read first line
    if (fgets(line, MAX_LINE_LENGTH, input)) {
        // Remove newline if present
        line[strcspn(line, "\n")] = 0;
        
        // Parse the line
        sscanf(line, "%s %s %s", label, opcode, operand);

        // Check if OPCODE is START
        if (strcmp(opcode, "START") == 0) {
            LOCCTR = (int)strtol(operand, NULL, 16);
            startingAddr = LOCCTR;
            
            // Add first label to symbol table if present
            if (strlen(label) > 0) {
                addSymbol(label, LOCCTR);
            }
            
            // Write to intermediate file
            fprintf(intermediate, "%-6d\t%04X\t%-8s\t%-8s\t%-8s\n", 
                    lineNumber, LOCCTR, label, opcode, operand);
        } else {
            LOCCTR = 0;
            startingAddr = 0;
        }

        // Process rest of the file
        while (fgets(line, MAX_LINE_LENGTH, input)) {
            lineNumber += 5;
            line[strcspn(line, "\n")] = 0;  // Remove newline
            
            // Clear previous values
            label[0] = opcode[0] = operand[0] = '\0';
            
            // Parse the line
            sscanf(line, "%s %s %s", label, opcode, operand);

            // Skip comment lines
            if (line[0] == '.') {
                fprintf(intermediate, "%-6d\t    \t        \t        \t        \t%s\n", 
                        lineNumber, line);
                continue;
            }

            // Write to intermediate file
            fprintf(intermediate, "%-6d\t%04X\t%-8s\t%-8s\t%-8s\n", 
                    lineNumber, LOCCTR, label, opcode, operand);

            // Add label to symbol table if present (only if it's in the leftmost column)
            if (strlen(label) > 0 && line[0] != ' ') {  // Check if label starts at beginning of line
                addSymbol(label, LOCCTR);
            }

            // Update LOCCTR based on instruction type
            if (strcmp(opcode, "WORD") == 0) {
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
                LOCCTR += 3;  // Standard instruction length
            }

            // Check for END
            if (strcmp(opcode, "END") == 0) {
                break;
            }
        }
    }

    // Print program length
    printf("\nProgram length: %04X\n", LOCCTR - startingAddr);

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

int main() {
    pass1("sicprog.txt", "intermediate.txt");
    return 0;
}

