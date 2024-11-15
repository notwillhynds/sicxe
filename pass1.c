#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables.h"
#include "symtab.h"

#define MAX_LINE_LENGTH 100
#define MAX_LABEL_LENGTH 20
#define MAX_OPCODE_LENGTH 10
#define MAX_OPERAND_LENGTH 20



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

    // // Write header to intermediate file
    // fprintf(intermediate, "%-6s\t%-8s\t%-8s\t%-8s\t%-8s\t%s\n",
    //         "Line", "Address", "Label", "OPCODE", "OPERAND", "Comment");
    // fprintf(intermediate, "------------------------------------------------------------------------------\n");

    // Read first line
    if (fgets(line, MAX_LINE_LENGTH, input)) {
        // Remove newline if present
        line[strcspn(line, "\n")] = 0;
        
        // Parse the line
        int numFields = sscanf(line, "%s %s %s", label, opcode, operand);

        // Handle cases based on the number of fields parsed
        if (numFields == 3) {
            // Line has label, opcode, and operand
        } else if (numFields == 2) {
            // Line has opcode and operand, no label
            strcpy(opcode, label);  // Move opcode to correct variable
            strcpy(operand, opcode);
            label[0] = '\0';  // Clear label
        } else if (numFields == 1) {
            // Line has only opcode, no label or operand
            strcpy(opcode, label);  // Move opcode to correct variable
            label[0] = '\0';  // Clear label
            operand[0] = '\0';  // Clear operand
        } else {
            // Handle error or comment line
        }

        // Check if OPCODE is START
        if (strcmp(opcode, "START") == 0) {
            LOCCTR = (int)strtol(operand, NULL, 16);
            startingAddr = LOCCTR;
            
            // Add first label to symbol table if present
            if (strlen(label) > 0 && searchSymTab(label) == -1) {
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

                // Skip comment lines (check this BEFORE parsing)
            if (line[0] == '.' || strstr(line, "\t.") != NULL) {
            fprintf(intermediate, "%-6d\t    \t        \t        \t        \t%s\n", 
                lineNumber, line);
                continue;  // Skip to next line
    }
            
            // Clear previous values
            label[0] = opcode[0] = operand[0] = '\0';
            
            // Check if the line starts with a space (indicating no label)
            if (line[0] == ' ' || line[0] == '\t') {
                // No label, parse opcode and operand
                sscanf(line, "%s %s", opcode, operand);
            } else {
                // Parse label, opcode, and operand
                sscanf(line, "%s %s %s", label, opcode, operand);
            }

            // Debugging output to verify parsing
            printf("Parsed line: Label='%s', Opcode='%s', Operand='%s'\n", label, opcode, operand);

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
            if (strlen(label) > 0 && line[0] != ' ' && searchSymTab(label) == -1) {  // Check if label starts at beginning of line
                addSymbol(label, LOCCTR);
            }

            // Update LOCCTR based on instruction type
            if(searchOpTab(opcode) == 3) {
                printf("Opcode %s Found at: %04X\n", opcode, LOCCTR);
                LOCCTR += 3;  // Standard instruction length
            }
            else if (strcmp(opcode, "WORD") == 0) {
                LOCCTR += 3;
            }
            else if (strcmp(opcode, "RESW") == 0) {
                LOCCTR += 3 * strtol(operand, NULL, 10);
            }
            else if (strcmp(opcode, "RESB") == 0) {
                LOCCTR += (int)strtol(operand, NULL, 10);
            }
            else if (strcmp(opcode, "BYTE") == 0) {
                if (operand[0] == 'C') {
                    LOCCTR += strlen(operand) - 3;  // Subtract 3 for C''
                }
                else if (operand[0] == 'X') {
                    LOCCTR += (strlen(operand) - 3) / 2;  // Subtract 3 for X''
                }
            }


            // Check for END
            if (strcmp(opcode, "END") == 0) {
                break;
            }
        }
    }
    // Print program length
    printf("\nProgram length: %04X\n", LOCCTR - startingAddr);
    // Write symbol table to file
    FILE* symtabFile = fopen("symtab.txt","w");
    for (int i = 0; i < symCount; i++) {
        fprintf(symtabFile, "%-6s\t%04X\n", SYMTAB[i].label, SYMTAB[i].address);
    }
    fclose(symtabFile);

    fclose(input);
    fclose(intermediate);
}



