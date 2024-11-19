#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables.h"
#include "symtab.h"
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1000
#define MARKER '|'
#define MAX_TEXT_RECORD_LENGTH 30

//Tokenize Intermediate File Lines
void pass2_revised_v1(char* intermediate) {
    FILE* file = fopen(intermediate, "r");
    FILE* listing = fopen("listing.txt", "w");
    FILE* object = fopen("objectcode.txt", "w");
    if(file == NULL || listing == NULL || object == NULL) {
        printf("Error opening file\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    char programName[7] = "";
    char lineNum[50], address[50], label[50], opcode[50], operand[50];
    unsigned int objcode = 0; //objectcode for listing file
    char* objectFCode = NULL;  //object code for obbjectfile

    int objCodeCapacity = 500;  //
    int lineCount = 0;
    SYMBOL symtab[MAX_SYMBOLS];
    int symCount = 0;
    int endingAddr = 0;
    int startingAddr = 0;

    //Save position for header
    long headerPos = ftell(object);
    //Write placeholder header
    fprintf(object, "H                                \n");
    
    //Variables for text record handling
    int recordLength = 0;  // Length in bytes
    char recordBuffer[140] = "";  // Buffer for hex chars and separators
    int currentTextStart = 0;

    //Store Object Code to write to object file initial capacity 100 bytes.
    objectFCode = malloc(objCodeCapacity * sizeof(char));
    if(objectFCode == NULL) {
        printf("Memory Allocation Failed\n");
        return;
    }

    loadSymbolTable("symtab.txt", symtab, &symCount);
    
    // Process first line to capture header info
    if(fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %s %s %s %s", lineNum, address, label, opcode, operand);
        if(strcmp(opcode, "START") == 0) {
            strncpy(programName, label, 6);  // Save program name
            programName[6] = '\0';           // Ensure null termination
            startingAddr = (int)strtol(operand, NULL, 16);
            printf("Name: %s\n", programName);
        }
    }
    rewind(file);

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

        // Capture starting address when START directive is found
        if(strcmp(opcode, "START") == 0) {
            startingAddr = (int)strtol(address, NULL, 16);
            printf("Starting Addr: %06X\n", startingAddr);
        }
        // Capture ending address from the last address before END
        else if(strcmp(opcode, "END") == 0) {
            endingAddr = (int)strtol(address, NULL, 16);
        }

        // Skip object code generation for directives
        if (strcmp(opcode, "START") == 0 || 
            strcmp(opcode, "END") == 0 || 
            strcmp(opcode, "RESW") == 0 || 
            strcmp(opcode, "RESB") == 0) {
            // Write current record if we have one
            if (recordLength > 0) {
                fprintf(object, "T%06X^%02X%s\n", currentTextStart, recordLength, recordBuffer);
                recordBuffer[0] = '\0';
                recordLength = 0;
            }
              // Skip to next instruction
        }

        int codeLength = 0;

         //Generate Object Codes
        if(strcmp(opcode, "RSUB") == 0) {
            objcode = 0x4C0000;
            codeLength = 3;
        }
	else if(opcode[0] == '+') {
            unsigned int opcodeValue = getOpcode(opcode + 1);
            bool immediate = (operand[0] == '#');
            bool indirect = (operand[0] == '@');
            char symbol[50];
            strcpy(symbol, operand + (immediate || indirect ? 1 : 0));

            unsigned int symbolAddr = getSymbolAddress(symbol, symtab, &symCount);
            if (symbolAddr == (unsigned int)-1) {
                printf("Undefined symbol: %s\n", symbol);
                exit(1);
            }

            unsigned int niBits = immediate ? 0x01 : (indirect ? 0x02 : 0x03);
            objcode = (opcodeValue << 24) | (niBits << 22) | symbolAddr;
            codeLength = 4;
	}
        else if(searchOpTab(opcode) == 3) { // Assuming searchOpTab returns format type
            unsigned int opcodeValue = getOpcode(opcode);
            char symbol[50];
            bool immediate = (operand[0] == '#');
            bool indirect = (operand[0] == '@');
            bool indexed = false;

            // Check for indexed addressing ',X'
            char *comma = strchr(operand, ',');
            if(comma != NULL) {
                // Extract symbol name before ','
                int len = comma - operand;
                if(len >= sizeof(symbol)) {
                    printf("Symbol name too long: %s\n", operand);
                    exit(1);
                }
                strncpy(symbol, operand, len);
                symbol[len] = '\0';
                indexed = true;
            }
            else {
                strcpy(symbol, operand);
            }

            unsigned int symbolAddr = getSymbolAddress(symbol, symtab, &symCount);
            if(symbolAddr == (unsigned int)-1) { // Assuming getSymbolAddress returns -1 for undefined symbols
                printf("Undefined symbol: %s\n", symbol);
                // Handle undefined symbol appropriately (e.g., exit or assign a default value)
                exit(1);
            }

            if(indexed) {
                 symbolAddr = 0x8000; // Set the indexed addressing bit (bit 15)
            }

            unsigned int niBits = immediate ? 0x01 : (indirect ? 0x02 : 0x03);
            unsigned int xbpeBits = (indexed ? 0x08 : 0) | 0x02; // PC-relative

            unsigned int displacement = symbolAddr - ((int)strtol(address, NULL, 16) + 3);
            if (displacement < -2048 || displacement > 2047) {
                printf("Displacement out of range: %d\n", displacement);
                exit(1);
            }
        }
        else if(strcmp(opcode, "BYTE") == 0) {
            if(operand[0] == 'C') {
                objcode = 0;
                for(int i = 2; operand[i] != '\''; i++) {
                    objcode = (objcode << 8) | operand[i];
                }
                codeLength = strlen(operand) - 3;  // Length of characters
            }
            else if(operand[0] == 'X') {
                sscanf(&operand[2], "%6X", &objcode);
                codeLength = 1;  // Hex constant is 1 byte
                // Restrict hex constants to two bytes
                objcode &= 0xFFFF;  // Mask to ensure only two bytes
            }
        }
        else if(strcmp(opcode, "WORD") == 0) {
            objcode = atoi(operand);
            codeLength = 3;
        }

        // Handle text records
        if (recordLength + codeLength > MAX_TEXT_RECORD_LENGTH) {
            // Write current record
            if (recordLength > 0) {
                fprintf(object, "T%06X^%02X%s\n", currentTextStart, recordLength, recordBuffer);
                recordBuffer[0] = '\0';
                recordLength = 0;
            }
            // Start new record at current address
            currentTextStart = (int)strtol(address, NULL, 16);
        }

	// Changes Below

        char objCodeStr[20];
        sprintf(objCodeStr, "^%0*X", codeLength * 2, objcode);
        strcat(recordBuffer, objCodeStr);
        recordLength += codeLength;

        lineCount++;
        fprintf(listing, "%-6s  %-8s %-8s  %-8s %0*X\n",
                address, label, opcode, operand, codeLength * 2, objcode);

        

    // Write final record if any remains
    if (recordLength > 0) {
        fprintf(object, "T%06X^%02X%s\n", currentTextStart, recordLength, recordBuffer);
    }

    // Write end record
    fprintf(object, "E%06X\n", startingAddr);

    // Go back and write the header
    fseek(object, headerPos, SEEK_SET);
    fprintf(object, "H^%-6s%06X^%06X", programName, startingAddr, endingAddr - startingAddr);

    // Clean up
    fclose(file);
    fclose(listing);
    fclose(object);

    
}

}
