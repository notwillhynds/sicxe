#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables.h"
#include "symtab.h"

#define MAX_LINE_LENGTH 100
#define MAX_LABEL_LENGTH 20
#define MAX_OPCODE_LENGTH 10
#define MAX_OPERAND_LENGTH 20




void pass1_revised_v3(const char *inputFile, const char *intermediateFile)
{
    FILE *input = fopen(inputFile, "r");
    FILE *intermediate = fopen(intermediateFile, "w");

    if (input == NULL || intermediate == NULL)
    {
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

    if (fgets(line, MAX_LINE_LENGTH, input))
    {

        // Remove newline if present
        line[strcspn(line, "\n")] = 0;

        // Initial parsing logic remains the same
        // Parse The line
        int numFields = sscanf(line, "%s %s %s", label, opcode, operand);

        // Handle cases based on the number of fields parsed
        if (numFields == 3)
        {
            // Line has label, opcode, and operand
        }
        else if (numFields == 2)
        {
            // Line has opcode and operand, no label
            strcpy(opcode, label); // Move opcode to correct variable
            strcpy(operand, opcode);
            label[0] = '\0'; // Clear label
        }
        else if (numFields == 1)
        {
            // Line has only opcode, no label or operand
            strcpy(opcode, label); // Move opcode to correct variable
            label[0] = '\0';       // Clear label
            operand[0] = '\0';     // Clear operand
        }
        else
        {
            // Handle error or comment line
        }

        // START directive handling remains the same
        // Check if OPCODE is START
        if (strcmp(opcode, "START") == 0)
        {
            LOCCTR = (int)strtol(operand, NULL, 16);
            startingAddr = LOCCTR;

            // Add first label to symbol table if present
            if (strlen(label) > 0 && searchSymTab(label) == -1)
            {
                addSymbol(label, LOCCTR);
            }

            // Write to intermediate file
            fprintf(intermediate, "%-6d\t%04X\t%-8s\t%-8s\t%-8s\n",
                    lineNumber, LOCCTR, label, opcode, operand);
        }
        else
        {
            LOCCTR = 0;
            startingAddr = 0;
        }

        // Process rest of the file
        while (fgets(line, MAX_LINE_LENGTH, input))
        {
            lineNumber += 5;
            line[strcspn(line, "\n")] = 0; // Remove newline

            // Skip comment lines (check this BEFORE parsing)
            if (line[0] == '.' || strstr(line, "\t.") != NULL)
            {
                fprintf(intermediate, "%-6d\t    \t        \t        \t        \t%s\n",
                        lineNumber, line);
                continue; // Skip to next line
            }

            // Clear previous values
            label[0] = opcode[0] = operand[0] = '\0';

            // Check if the line starts with a space (indicating no label)
            if (line[0] == ' ' || line[0] == '\t')
            {
                // No label, parse opcode and operand
                sscanf(line, "%s %s", opcode, operand);
            }
            else
            {
                // Parse label, opcode, and operand
                sscanf(line, "%s %s %s", label, opcode, operand);
            }

            // Debugging output to verify parsing
            printf("Parsed line: Label='%s', Opcode='%s', Operand='%s'\n", label, opcode, operand);

            // Skip comment lines
            if (line[0] == '.')
            {
                fprintf(intermediate, "%-6d\t    \t        \t        \t        \t%s\n",
                        lineNumber, line);
                continue;
            }

            // Check if the opcode is an extended format
            int isExtendedFormat = (opcode[0] == '+');
            char processedOpcode[256];
            if (isExtendedFormat)
            {
                // Remove the '+' for opcode lookup
                memmove(opcode, opcode + 1, strlen(opcode));
            }

		//Edits start here, changed arguement in print statement from operand to formatted operand
		//This if statement below just preforms the operation of adding the # on in case its immediate addressing

            // Detect immediate addressing by checking for the '#' prefix
            int isImmediate = (operand[0] == '#');

            char formattedOperand[MAX_OPERAND_LENGTH + 1]; // Buffer for the formatted operand

            if (isImmediate) {
                 snprintf(formattedOperand, sizeof(formattedOperand), "#%s", operand); // Add '#' to the operand
            }
            else {
                 strcpy(formattedOperand, operand); // Copy operand as is
            }

            // Use the formatted operand in the fprintf statement
            fprintf(intermediate, "%-6d\t%04X\t%-8s\t%-8s\t%-8s\n",
                 lineNumber, LOCCTR, label, opcode, formattedOperand);

            // Add symbol to table if it has a label
            if (strlen(label) > 0 && line[0] != ' ' && searchSymTab(label) == -1 && !isImmediate)
            {
                addSymbol(label, LOCCTR);
            }

            // Update LOCCTR based on instruction type
            int opcodeFormat = searchOpTab(opcode);
            if (opcodeFormat == 3)
            {
                LOCCTR += isExtendedFormat ? 4 : 3; // 4 for extended, 3 for standard
            }
            else if (strcmp(opcode, "WORD") == 0)
            {
                LOCCTR += 3;
            }
            else if (strcmp(opcode, "RESW") == 0)
            {
                LOCCTR += 3 * strtol(operand, NULL, 10);
            }
            else if (strcmp(opcode, "RESB") == 0)
            {
                LOCCTR += (int)strtol(operand, NULL, 10);
            }
            else if (strcmp(opcode, "BYTE") == 0)
            {
                if (operand[0] == 'C')
                {
                    LOCCTR += strlen(operand) - 3; // Characters
                }
                else if (operand[0] == 'X')
                {
                    LOCCTR += (strlen(operand) - 3) / 2; // Hexadecimal
                }
            }

            // Check for END
            if (strcmp(opcode, "END") == 0)
            {
                break;
            }
        }
    }

    // Print program length and write symbol table (unchanged)
    printf("\nProgram length: %04X\n", LOCCTR - startingAddr);
    FILE *symtabFile = fopen("symtab.txt", "w");
    for (int i = 0; i < symCount; i++)
    {
        fprintf(symtabFile, "%-6s\t%04X\n", SYMTAB[i].label, SYMTAB[i].address);
    }
    fclose(symtabFile);

    fclose(input);
    fclose(intermediate);
}
