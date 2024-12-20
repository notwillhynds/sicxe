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
#define MAX_OPCODE_LENGTH 50
#define SIC 0
#define SICXE 1

//Tokenize Intermediate File Lines
void pass2(const char* intermediate, int format) {
    FILE* file = fopen(intermediate, "r");
    FILE* listing = fopen("listing.txt", "w");
    FILE* object = fopen("objectcode.txt", "w");
    if(file == NULL || listing == NULL || object == NULL) {
        printf("Error opening file\n");
        return;
    }

    printf("Processing as %s\n", format == SIC ? "SIC" : "SIC/XE");

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
               operand   // "1000"
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
        if (format == SIC) {
            // Handle SIC format instructions
            if(strcmp(opcode, "RSUB") == 0) {
                objcode = 0x4C0000;
                codeLength = 3;
            }
            else if(searchOpTab(opcode) == 3) {
                // Standard SIC format (no extended addressing)
                unsigned int opcodeValue = getOpcode(opcode);
                char symbol[50];
                bool indexed = false;

                // Check for indexed addressing ',X'
                char *comma = strchr(operand, ',');
                if(comma != NULL) {
                    int len = comma - operand;
                    strncpy(symbol, operand, len);
                    symbol[len] = '\0';
                    indexed = true;
                }
                else {
                    strcpy(symbol, operand);
                }

                unsigned int symbolAddr = getSymbolAddress(symbol, symtab, &symCount);
                if(symbolAddr == (unsigned int)-1) {
                    printf("Undefined symbol: %s\n", symbol);
                    exit(1);
                }

                if(indexed) {
                    symbolAddr |= 0x8000; // Set indexed bit
                }

                objcode = (opcodeValue << 16) | (symbolAddr & 0xFFFF);
                codeLength = 3;
            }
        }
        else if (format == SICXE) {
            // Handle SIC/XE format instructions
            if(opcode[0] == '+') {
                // Format 4 instruction
                codeLength = 4;
                char baseOpcode[MAX_OPCODE_LENGTH];
                strcpy(baseOpcode, opcode + 1);
                unsigned int opcodeValue = getOpcode(baseOpcode);
                
                // Set n and i bits for simple addressing (both 1)
                opcodeValue |= 0x3;  // Set ni bits to 11
                
                // Set e bit for format 4
                objcode = (opcodeValue << 24) | 0x100000;
                
                // Handle the address part
                unsigned int symbolAddr = getSymbolAddress(operand, symtab, &symCount);
                if(symbolAddr == (unsigned int)-1) {
                    printf("Undefined symbol: %s\n", operand);
                    exit(1);
                }
                
                objcode |= symbolAddr & 0xFFFFF; // Use 20 bits for address in format 4
            }
            else if(searchOpTab(opcode) == 2) {
                // Format 2 instruction
                codeLength = 2;
                unsigned int opcodeValue = getOpcode(opcode);
                objcode = opcodeValue << 8;
                
                // Parse register operands
                char reg1[10], reg2[10];
                if(strchr(operand, ',')) {
                    sscanf(operand, "%[^,],%s", reg1, reg2);
                    int r1 = getRegisterNum(&reg1[0]);  // Get first character as register name
                    int r2 = getRegisterNum(&reg2[0]);  // Get first character as register name
                    if(r1 >= 0 && r2 >= 0) {
                        objcode |= (r1 << 4) | r2;
                    } else {
                        printf("Error: Invalid register(s) in instruction\n");
                    }
                } else {
                    int r1 = getRegisterNum(&operand[0]);
                    if(r1 >= 0) {
                        objcode |= (r1 << 4);
                    } else {
                        printf("Error: Invalid register in instruction\n");
                    }
                }
            }
            else if(searchOpTab(opcode) == 3) {
                // Format 3 instruction
                codeLength = 3;
                unsigned int opcodeValue = getOpcode(opcode);
                
                // Set n and i bits for simple addressing (both 1)
                opcodeValue |= 0x3;  // Set ni bits to 11
                
                objcode = opcodeValue << 16;
                
                // Handle the address part
                if(strcmp(opcode, "RSUB") == 0) {
                    // RSUB doesn't need address bits
                    objcode |= 0;
                } else {
                    unsigned int symbolAddr = getSymbolAddress(operand, symtab, &symCount);
                    if(symbolAddr == (unsigned int)-1) {
                        printf("Undefined symbol: %s\n", operand);
                        exit(1);
                    }
                    
                    // For now, use direct addressing (will add PC-relative later)
                    objcode |= symbolAddr & 0xFFF; // Use 12 bits for address in format 3
                }
            }
        }

        // Handle BYTE, WORD, and other directives same as before
        if(strcmp(opcode, "BYTE") == 0) {
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

        // Add current object code to record
        if (recordLength == 0) {
            currentTextStart = (int)strtol(address, NULL, 16);
            if(strcmp(opcode, "BYTE") == 0 && operand[0] == 'X') {
                // Extract everything between the quotes
                char *start = strchr(operand, '\'') + 1;
                char *end = strrchr(operand, '\'');
                int len = end - start;
                char *hexStr = malloc(len + 1);
                strncpy(hexStr, start, len);
                hexStr[len] = '\0';
                sprintf(recordBuffer, "^%s", hexStr);
                free(hexStr);
            } else {
                sprintf(recordBuffer, "^%06X", objcode);
            }
        } else {
            char objCodeStr[20];  // Increased buffer size
            if(strcmp(opcode, "BYTE") == 0 && operand[0] == 'X') {
                // Extract everything between the quotes
                char *start = strchr(operand, '\'') + 1;
                char *end = strrchr(operand, '\'');
                int len = end - start;
                char *hexStr = malloc(len + 1);
                strncpy(hexStr, start, len);
                hexStr[len] = '\0';
                sprintf(objCodeStr, "^%s", hexStr);
                free(hexStr);
            } else {
                sprintf(objCodeStr, "^%06X", objcode);
            }
            strcat(recordBuffer, objCodeStr);
        }
        recordLength += codeLength;

        lineCount++;
        if(strcmp(opcode, "BYTE") == 0 && operand[0] == 'X') {
            // For hex BYTE constants, don't use leading zeros
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
            // For all other instructions, keep leading zeros
            printf("Line: %d Address: %-6s Label: %-8s Opcode: %-8s Operand: %-8s Obj Code: %06X\n",
                   lineCount, address, label, opcode, operand, objcode);
            fprintf(listing, "%-6s  %-8s %-8s  %-8s %06X\n",
                    address, label, opcode, operand, objcode);
        }



    }

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



