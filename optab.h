#ifndef OPCODE_H
#define OPCODE_H

int searchOpTab(const char* opcode);

typedef struct OpCode
{
    char mnemonic[7];
    unsigned char opcode;
}OPCODE;

#endif