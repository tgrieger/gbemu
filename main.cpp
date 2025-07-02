#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>

#include "opcode.h"
#include "register.h"

// TODO anywhere we are writing to memory, verify that the memory is actually writable.
unsigned char memory[0xFFFF];
uint16_t program_counter { 0x100 };
uint16_t stack_pointer {};
registers regs {};
bool ime { false };

uint16_t convert_bytes_to_word(const unsigned char high, const unsigned char low)
{
    return static_cast<uint16_t>(low) | static_cast<uint16_t>(high) << 8;
}

unsigned char get_high_byte(const uint16_t word)
{
    return word >> 8 & 0xFF;
}

unsigned char get_low_byte(const uint16_t word)
{
    return word & 0xFF;
}

unsigned char load_full_register_from_memory(unsigned char &high, unsigned char &low)
{
    high = memory[program_counter + 2];
    low = memory[program_counter + 1];
    program_counter += 3;

    return 3;
}

unsigned char load_half_register_into_pointer(const unsigned char value, const unsigned char high, const unsigned char low)
{
    const uint16_t address = convert_bytes_to_word(high, low);
    memory[address] = value;
    program_counter++;

    return 2;
}

unsigned char increment_full_register(unsigned char &high, unsigned char &low)
{
    uint16_t full = convert_bytes_to_word(high, low);
    full++;
    high = get_high_byte(full);
    low = get_low_byte(full);

    program_counter++;

    return 2;
}

unsigned char increment_half_register(unsigned char &half)
{
    half++;
    set_z(regs.f, half == 0);
    set_n(regs.f, false);
    set_h(regs.f, (half & 0b00001111) == 0b00001111);
    program_counter++;

    return 1;
}

unsigned char decrement_half_register(unsigned char &half)
{
    half--;
    set_z(regs.f, half == 0);
    set_n(regs.f, true);
    set_h(regs.f, (half & 0b00001111) == 0b00001111);
    program_counter++;

    return 1;
}

unsigned char load_half_register_from_memory(unsigned char &half)
{
    half = memory[program_counter + 1];
    program_counter += 2;

    return 2;
}

unsigned char rotate_half_register_left(unsigned char &half)
{
    // NOTE - this doesn't use the typical way of setting the flags since three of them (z, n, and h) will always be set to 0.
    regs.f = 0b00010000 & half >> 3;
    half = half << 1 | get_c(regs.f);
    program_counter++;

    return 1;
}

unsigned char load_full_register_into_pointer(unsigned char high, unsigned char low)
{
    // TODO check if the target memory is actually writable
    uint16_t address = convert_bytes_to_word(memory[program_counter + 2], memory[program_counter + 1]);
    memory[address] = low;
    memory[address + 1] = high;
    program_counter += 3;

    // TODO this is for 0x08, is it the same for the others or is this one special?
    // TODO are there even others?
    return 5;
}

unsigned char run_op_code()
{
    switch (memory[program_counter])
    {
    case NO_OP:
        program_counter++;
        return 1;
    case LOAD_BC_FROM_MEMORY:
        return load_full_register_from_memory(regs.b, regs.c);
    case LOAD_A_INTO_BC_POINTER:
        return load_half_register_into_pointer(regs.a, regs.b, regs.c);
    case INCREMENT_BC:
        return increment_full_register(regs.b, regs.c);
    case INCREMENT_B:
        return increment_half_register(regs.b);
    case DECREMENT_B:
        return decrement_half_register(regs.b);
    case LOAD_B_FROM_MEMORY:
        return load_half_register_from_memory(regs.b);
    case ROTATE_A_LEFT:
        return rotate_half_register_left(regs.a);
    case LOAD_SP_INTO_POINTER:
        return load_full_register_into_pointer(get_high_byte(stack_pointer), get_low_byte(stack_pointer));
    case ADD_BC_TO_HL:
        // TODO is there a way to determine the half carry and carry bits by just doing the math normally?
        {
            uint16_t bc = convert_bytes_to_word(regs.b, regs.c);
            uint16_t hl = convert_bytes_to_word(regs.h, regs.l);

            uint32_t result = bc + hl;
            hl = result & 0xFFFF;

            regs.h = get_high_byte(hl);
            regs.l = get_low_byte(hl);

            // While I normally use binary, using hex here for brevity since it's 16 bits.
            set_c(regs.f, result > 0xFFFF);
            set_h(regs.f, (hl & 0x0FFF) + (bc & 0x0FFF) > 0x0FFF);
        }

        set_n(regs.f, false);
        program_counter++;
        break;
    case LOAD_BC_POINTER_INTO_A:
        regs.a = memory[convert_bytes_to_word(regs.b, regs.c)];
        program_counter++;
        break;
    case DECREMENT_BC:
        {
            uint16_t bc = convert_bytes_to_word(regs.b, regs.c);
            bc--;
            regs.b = get_high_byte(bc);
            regs.c = get_low_byte(bc);
        }

        program_counter++;
        break;
    case INCREMENT_C:
        regs.c++;
        set_z(regs.f, regs.c == 0);
        set_n(regs.f, false);
        set_h(regs.f, (regs.c & 0b00001111) == 0b00001111);
        program_counter++;
        break;
    case DECREMENT_C:
        regs.c--;
        set_z(regs.f, regs.c == 0);
        set_n(regs.f, true);
        set_h(regs.f, (regs.c & 0b00001111) == 0b00001111);
        program_counter++;
        break;
    case LOAD_C_FROM_MEMORY:
        regs.c = memory[program_counter + 1];
        program_counter += 2;
        break;
    case ROTATE_A_RIGHT:
        // NOTE - this doesn't use the typical way of setting the flags since three of them (z, n, and h) will always be set to 0.
        regs.f = 0b00010000 & regs.a << 4;
        regs.a = regs.a >> 1 | get_c(regs.f) << 7;
        program_counter++;
        break;
    case STOP:
        // TODO implement stop
        program_counter += 2;
        break;
    case LOAD_DE_FROM_MEMORY:
        regs.d = memory[program_counter + 2];
        regs.e = memory[program_counter + 1];
        program_counter += 3;
        break;
    case LOAD_A_INT_DE_POINTER:
        {
            const uint16_t address = convert_bytes_to_word(regs.d, regs.e);
            memory[address] = regs.a;
        }

        program_counter++;
        break;
    case JUMP_IF_Z_IS_ZERO:
        // If the z flag is 0, jump some number of bytes based on the next byte
        if (!get_z(regs.f))
        {
            // TODO learn more about pros and cons of static_cast
            program_counter += static_cast<char>(memory[program_counter + 1]);
        }

        program_counter += 2;
        break;
    case LOAD_HL_FROM_MEMORY:
        regs.l = memory[program_counter + 1];
        regs.h = memory[program_counter + 2];
        program_counter += 3;
        break;
    case LOAD_SP_FROM_MEMORY:
        stack_pointer = convert_bytes_to_word(memory[program_counter + 2], memory[program_counter + 1]);
        program_counter += 3;
        break;
    case LOAD_A_INTO_HL_CONTENTS_AND_DECREMENT_HL:
        {
            uint16_t hl = convert_bytes_to_word(regs.h, regs.l);
            memory[hl] = regs.a;

            // Decrement the whole of hl before breaking it back into its parts
            hl--;
            regs.h = get_high_byte(hl);
            regs.l = get_low_byte(hl);

            program_counter++;
        }
        break;
    case XOR_A:
        // This case will always result in A being 0, no need to actually XOR A with A
        // regs.a = regs.a ^ regs.a;
        regs.a = 0;

        // Because A will always be 0, we will always clear all flags except Z and set Z to 1
        /*
        regs.f = 0;
        if (regs.a == 0)
        {
            // Set Z to 1 if the result of the XOR was 0
            regs.f |= 0b00000001;
        }
        */

        // NOTE - this doesn't use the typical set method to set z to 1 and all other flags to 0
        regs.f = 0b10000000;

        program_counter++;
        break;
    case JUMP:
        program_counter = convert_bytes_to_word(memory[program_counter + 2], memory[program_counter + 1]);
        break;
    case LOAD_A_FROM_LOW_POINTER:
        {
            uint16_t address = convert_bytes_to_word(0xFF, memory[program_counter + 1]);
            regs.a = memory[address];
        }

        program_counter += 2;
        break;
    case DISABLE_IME:
        ime = false;
        program_counter++;
        break;
    case LOAD_A_FROM_POINTER:
        regs.a = memory[convert_bytes_to_word(memory[program_counter + 2], memory[program_counter + 1])];
        program_counter += 3;
        break;
    case COMPARE_A_TO_MEMORY:
        set_n(regs.f, true);
        set_z(regs.f, regs.a == memory[program_counter + 1]);

        // TODO understand how the half carry and carry flags are being set better
        set_h(regs.f, (regs.a & 0x0F) < (memory[program_counter + 1] & 0x0F));
        set_c(regs.f, regs.a < memory[program_counter + 1]);

        program_counter += 2;
        break;
    case USE_EXTENDED_OP_CODE:
        switch (memory[program_counter + 1])
        {
        case COPY_INVERSE_BIT_7_H_TO_Z:
            // Set the h flag to 1
            set_h(regs.f, true);

            // Set the c flag to 0
            set_c(regs.f, false);

            // Set the z flag to the compliment of the 7th bit of the h register
            // General formula for setting the nth bit to x: `number = number & ~(1 << n) | (x << n)`
            set_z(regs.f, ~(regs.h >> 7) & 0b00000001);
            break;
        default:
            std::cerr << "Unknown extended opcode: 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(memory[program_counter + 1]) << " at pc 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(program_counter + 1) << '\n';
            throw std::runtime_error("Unknown extended opcode: 0x");
        }
        program_counter += 2;
        break;
    default:
        std::cerr << "Unknown opcode: 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(memory[program_counter]) << " at pc 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(program_counter) << '\n';
        throw std::runtime_error("Unknown opcode: 0x");
    }

    // TODO remove this once all cases return
    return 0;
}

int main()
{
    FILE *file;
    if (const errno_t error = fopen_s(&file, "C:\\Users\\tgrieger\\Source\\repos\\gbemu\\artifacts\\bgbtest.gb", "rb"))
    {
        std::cout << error << std::endl;
    }

    // Read all contents of the file into the memory buffer
    int pos = 0;
    while (fread(&memory[pos], 1, 1, file))
    {
        pos++;
    }

    fclose(file);

    while (true)
    {
        run_op_code();
    }
}
