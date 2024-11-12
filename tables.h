#ifndef OPCODE_H
#define OPCODE_H

int searchOpTab(const char* opcode);
unsigned int getOpcode(const char* mnemonic);

typedef struct OpCode
{
    char mnemonic[7];
    unsigned char opcode;
}OPCODE;

#endif