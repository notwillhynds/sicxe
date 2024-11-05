#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100
#define MAX_LABEL_LENGTH 20
#define MAX_OPCODE_LENGTH 10
#define MAX_OPERAND_LENGTH 20

void processFirstLine(const char *sourceFilename, const char *intermediateFilename) {
    FILE *sourceFile = fopen(sourceFilename, "r");
    FILE *intermediateFile = fopen(intermediateFilename, "w");
    if (sourceFile == NULL || intermediateFile == NULL) {
        printf("Error opening files.\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    char label[MAX_LABEL_LENGTH], opcode[MAX_OPCODE_LENGTH], operand[MAX_OPERAND_LENGTH];
    int LOCCTR = 0;

    // Read the first line
    if (fgets(line, MAX_LINE_LENGTH, sourceFile) != NULL) {
        // Parse the line into label, opcode, and operand
        sscanf(line, "%s %s %s", label, opcode, operand);

        // Check if the opcode is "START"
        if (strcmp(opcode, "START") == 0) {
            // Save the operand as the starting address
            LOCCTR = (int)strtol(operand, NULL, 16);
            fprintf(intermediateFile, "%s", line); // Write line to intermediate file
            printf("Opcode is START. Starting address: %X\n", LOCCTR);

            // Read the next input line
            if (fgets(line, MAX_LINE_LENGTH, sourceFile) != NULL) {
                printf("Next line: %s", line);
            }
        } else {
            // Initialize LOCCTR to 0
            LOCCTR = 0;
            printf("Opcode is not START. LOCCTR initialized to 0.\n");
        }
    } else {
        printf("Error reading the first line.\n");
    }

    fclose(sourceFile);
    fclose(intermediateFile);
}

int main() {
    processFirstLine("sicprog.txt", "intermediate.txt");
    return 0;
}

