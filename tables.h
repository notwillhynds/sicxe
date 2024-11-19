#ifndef OPCODE_H
#define OPCODE_H

int searchOpTab(const char* opcode);
unsigned int getOpcode(const char* mnemonic);
int getRegisterNum(const char* regName);

typedef struct OpCode
{
    char mnemonic[7];
    unsigned char opcode;
    int format; 
}OPCODE;

typedef struct Regs {
    char mnemonic[2];
    int num;
}REG;

#endif