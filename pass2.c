#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables.h"
#include "symtab.h"

#define MAX_LINE_LENGTH 1000

//Tokenize Intermediate File Lines
void pass2(char* intermediate) {
    FILE* file = fopen(intermediate, "r");
    FILE* listing = fopen("listing.txt", "w");
    if(file == NULL || listing == NULL) {
        printf("Error opening file\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    char lineNum[50], address[50], label[50], opcode[50], operand[50];
    unsigned int objcode = 0;
    int lineCount = 0;
    SYMBOL symtab[MAX_SYMBOLS];
    int symCount = 0;

    loadSymbolTable("symtab.txt", symtab, &symCount);

    while(fgets(line, sizeof(line), file)){
        //Reset all strings
        lineNum[0] = address[0] = label[0] = opcode[0] = operand[0] = '\0';
        //Parse Lines
           int fields = sscanf(line, "%s %s %s %s %s", 
               lineNum,   // "5"
               address,   // "1000"
               label,     // "COPY"
               opcode,    // "START"
               operand    // "1000"
        );
        //Skip Comments
        if (address[0] == '.' ) continue;

        //Fix Column alignment so tokens are correct
        if(fields == 4) { // No Label
            strcpy(operand, opcode);
            strcpy(opcode,label);
            label[0] = '\0';
        }
        else if(fields == 3) { //No Operand
            strcpy(opcode, label);
            label[0] = '\0';
        }


         
        if(strcmp(opcode, "RSUB") == 0) {
            objcode = 0x4C0000;  // RSUB is always 4C0000
        }
        else if(searchOpTab(opcode) == 3) {
            unsigned int opcodeValue = getOpcode(opcode);
            unsigned int symbolAddr;

            // Check if operand has indexed addressing mode
            char* indexedOperand = strchr(operand, ',');
            if(indexedOperand != NULL) {
                // Remove the ",X" part from the operand
                *indexedOperand = '\0';
                symbolAddr = getSymbolAddress(operand, symtab, &symCount);
                // Set the indexed addressing flag in the object code
                objcode = (opcodeValue << 16) | (symbolAddr & 0xFFFF) | 0x8000;
            }
            else {
                symbolAddr = getSymbolAddress(operand, symtab, &symCount);
                if(symbolAddr != 0xFFFF) {
                    objcode = (opcodeValue << 16) | (symbolAddr & 0xFFFF);
                }
            }
        }
        else if(strcmp(opcode, "BYTE") == 0) {
            if(operand[0] == 'C') {
                // Handle character constant
                objcode = 0;
                for(int i = 2; operand[i] != '\''; i++) {
                    objcode = (objcode << 8) | operand[i];
                }
            }
            else if(operand[0] == 'X') {
                // Handle hex constant - extract just the hex value
                char hex[3];
                strncpy(hex, &operand[2], 2);  // Copy just the hex digits
                hex[2] = '\0';
                unsigned int hexValue;
                sscanf(hex, "%x", &hexValue);
                objcode = hexValue;  // Store just the hex value without padding
            }
        }
        else if(strcmp(opcode, "WORD") == 0) {
            objcode = atoi(operand);
        }
        else {
            objcode = 0;  // For START, END, RESW, RESB
        }

        lineCount++;
        if(strcmp(opcode, "BYTE") == 0 && operand[0] == 'X') {
            // For hex BYTE constants
            printf("Line: %d Address: %-6s Label: %-8s Opcode: %-8s Operand: %-8s Obj Code: %02X\n",
                   lineCount, address, label, opcode, operand, objcode & 0xFF);
            fprintf(listing, "%-6s  %-8s %-8s  %-8s %02X\n",
                    address, label, opcode, operand, objcode & 0xFF);
        }
        else if(strcmp(opcode, "RESW") == 0 || strcmp(opcode, "RESB") == 0 || 
                strcmp(opcode, "START") == 0 || strcmp(opcode, "END") == 0) {
            // For RESW, RESB, START, and END - print with no object code
            printf("Line: %d Address: %-6s Label: %-8s Opcode: %-8s Operand: %-8s\n",
                   lineCount, address, label, opcode, operand);
            fprintf(listing, "%-6s  %-8s %-8s  %-8s\n",
                    address, label, opcode, operand);
        }
        else {
            // For all other instructions
            printf("Line: %d Address: %-6s Label: %-8s Opcode: %-8s Operand: %-8s Obj Code: %06X\n",
                   lineCount, address, label, opcode, operand, objcode);
            fprintf(listing, "%-6s  %-8s %-8s  %-8s %06X\n",
                    address, label, opcode, operand, objcode);
        }
               
    }
    fclose(file);
    fclose(listing);
}

int main() {

    pass2("intermediate.txt");
    }
