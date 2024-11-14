#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables.h"
#include "symtab.h"
#include <string.h>

#define MAX_LINE_LENGTH 1000

//Tokenize Intermediate File Lines
void pass2(char* intermediate) {
    FILE* file = fopen(intermediate, "r");
    FILE* listing = fopen("listing.txt", "w");
     FILE* object = fopen("objectcode.txt", "w");
    if(file == NULL || listing == NULL) {
        printf("Error opening file\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    char programName[7] = "";
    char lineNum[50], address[50], label[50], opcode[50], operand[50];
    unsigned int objcode = 0; //objectcode for listing file
    char* objectFCode = NULL;  //object code for obbjectfile
    int objCodeSize = 0;
    int objCodeCapacity = 100;
    int lineCount = 0;
    SYMBOL symtab[MAX_SYMBOLS];
    int symCount = 0;
    int endingAddr = 0;
    int startingAddr = 0;

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


        int codeLength = 0;

         //Generate Object Codes
        if(strcmp(opcode, "RSUB") == 0) {
            objcode = 0x4C0000;
            
            if(codeLength == 30) {
                // Write current text record
                fprintf(object, "T%06X%02X\n", startingAddr, codeLength);
                for(int i = 0; i < objCodeSize; i += 3) {
                    unsigned int fullCode = (objectFCode[i] << 16) | 
                                          (objectFCode[i+1] << 8) | 
                                          objectFCode[i+2];
                    fprintf(object, "%06X", fullCode);
                }
                fprintf(object, "\n");
                
                codeLength = 0;
                objCodeSize = 0;
            }
            
            if(codeLength + 3 <= 30 && objCodeSize + 3 < objCodeCapacity) {
                objectFCode[objCodeSize] = 0x4C;     
                objectFCode[objCodeSize + 1] = 0x00; 
                objectFCode[objCodeSize + 2] = 0x00; 
                objCodeSize += 3;
                codeLength += 3;
            }
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

                if(codeLength == 30) {
                // Write current text record
                fprintf(object, "T%06X%02X\n", startingAddr, codeLength);
                for(int i = 0; i < objCodeSize; i += 3) {
                    unsigned int fullCode = (objectFCode[i] << 16) | 
                                          (objectFCode[i+1] << 8) | 
                                          objectFCode[i+2];
                    fprintf(object, "%06X", fullCode);
                }
                fprintf(object, "\n");
                
                codeLength = 0;
                objCodeSize = 0;
            }
                
                // Store in object code array if within limits
                if(codeLength + 3 <= 30 && objCodeSize + 3 < objCodeCapacity) {
                    objectFCode[objCodeSize] = (objcode >> 16) & 0xFF;     // First byte (opcode)
                    objectFCode[objCodeSize + 1] = (objcode >> 8) & 0xFF;  // Second byte (addr high)
                    objectFCode[objCodeSize + 2] = objcode & 0xFF;         // Third byte (addr low)
                    objCodeSize += 3;
                    codeLength += 3;
                }
            }
            else {
                symbolAddr = getSymbolAddress(operand, symtab, &symCount);
                if(symbolAddr != 0xFFFF) {
                    objcode = (opcodeValue << 16) | (symbolAddr & 0xFFFF);

                                if(codeLength == 30) {
                // Write current text record
                fprintf(object, "T%06X%02X\n", startingAddr, codeLength);
                for(int i = 0; i < objCodeSize; i += 3) {
                    unsigned int fullCode = (objectFCode[i] << 16) | 
                                          (objectFCode[i+1] << 8) | 
                                          objectFCode[i+2];
                    fprintf(object, "%06X", fullCode);
                }
                fprintf(object, "\n");
                
                codeLength = 0;
                objCodeSize = 0;
            }
                    
                    // Store in object code array if within limits
                    if(codeLength + 3 <= 30 && objCodeSize + 3 < objCodeCapacity) {
                        objectFCode[objCodeSize] = (objcode >> 16) & 0xFF;     // First byte (opcode)
                        objectFCode[objCodeSize + 1] = (objcode >> 8) & 0xFF;  // Second byte (addr high)
                        objectFCode[objCodeSize + 2] = objcode & 0xFF;         // Third byte (addr low)
                        objCodeSize += 3;
                        codeLength += 3;
                    }
                }
            }
        }
            else if(strcmp(opcode, "BYTE") == 0) {
                if(operand[0] == 'C') {
                    // Handle character constant (e.g., C'EOF')
                    objcode = 0;
                    int byteCount = 0;  // Count how many characters
                    
                    // Calculate number of bytes and build objcode
                    for(int i = 2; operand[i] != '\''; i++) {
                        byteCount++;
                    }
                    
                    // Check if we need to start new text record
                    if(codeLength + byteCount > 30) {
                        // Write current text record
                        fprintf(object, "T%06X%02X", startingAddr, codeLength);
                        for(int i = 0; i < objCodeSize; i += 3) {
                            unsigned int fullCode = (objectFCode[i] << 16) | 
                                                (objectFCode[i+1] << 8) | 
                                                objectFCode[i+2];
                            fprintf(object, "%06X", fullCode);
                        }
                        fprintf(object, "\n");
                        
                        codeLength = 0;
                        objCodeSize = 0;
                    }
                    
                    // Store each character in objectFCode
                    if(codeLength + byteCount <= 30) {
                        for(int i = 2; operand[i] != '\''; i++) {
                            objectFCode[objCodeSize] = operand[i];  // Store ASCII value
                            objCodeSize++;
                            codeLength++;
                        }
                    }
                    
                    // For listing file
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


    // First write header record
    fprintf(object, "H%-6s%06X%06X\n", programName, startingAddr, endingAddr - startingAddr);

    // Write text records
    int currentPos = 0;
    while(currentPos < objCodeSize) {
        // Calculate how many bytes remain
        int remainingBytes = objCodeSize - currentPos;
        // Determine length of this text record (max 30 bytes)
        int recordLength = (remainingBytes >= 30) ? 30 : remainingBytes;
        
        // Write text record header
        fprintf(object, "T%06X%02X", startingAddr + (currentPos/3), recordLength);
        
        // Write object codes for this record
        for(int i = currentPos; i < currentPos + recordLength; i += 3) {
            unsigned int fullCode = (objectFCode[i] << 16) | 
                                  (objectFCode[i+1] << 8) | 
                                   objectFCode[i+2];
            fprintf(object, "%06X", fullCode);
        }
        fprintf(object, "\n");
        
        // Move to next position
        currentPos += recordLength;
    }

    // Write end record
    fprintf(object, "E%06X\n", startingAddr);

    fclose(file);
    fclose(listing);
    fclose(object);

    
}

int main() {

    pass2("intermediate.txt");
    }