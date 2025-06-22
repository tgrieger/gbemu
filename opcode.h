#ifndef OPCODES_H
#define OPCODES_H

enum op_code : unsigned char
{
    NO_OP = 0x00,
    JUMP_IF_Z_IS_ZERO = 0x20,
    LOAD_HL_FROM_MEMORY = 0x21,
    LOAD_SP_FROM_MEMORY = 0x31,
    LOAD_A_INTO_HL_CONTENTS_AND_DECREMENT_HL = 0x32,
    SUBTRACT_C_INTO_A = 0x9F,
    XOR_A = 0xAF,
    USE_EXTENDED_OP_CODE = 0xCB,
    LOAD_A_FROM_POINTER = 0xFA
};

enum extended_op_code
{
    COPY_INVERSE_BIT_7_H_TO_Z = 0x7C
};

#endif //OPCODES_H
