#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>

#include "opcode.h"
#include "register.h"

uint16_t convert_bytes_to_word(unsigned char low, unsigned char high)
{
    return static_cast<uint16_t>(low) | static_cast<uint16_t>(high) << 8;
}

int main()
{
    unsigned char memory[0xFFFF];
    FILE *file;
    errno_t error = fopen_s(&file, "C:\\Users\\tgrieger\\Source\\repos\\gbemu\\artifacts\\bgbtest.gb", "rb");
    if (error)
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

    uint16_t program_counter { 0x100 };
    uint16_t stack_pointer {};
    registers regs {};
    bool ime { false };
    while (true)
    {
        switch (memory[program_counter])
        {
        case NO_OP:
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
            stack_pointer = convert_bytes_to_word(memory[program_counter + 1], memory[program_counter + 2]);
            program_counter += 3;
            break;
        case LOAD_A_INTO_HL_CONTENTS_AND_DECREMENT_HL:
            {
                uint16_t hl = convert_bytes_to_word(regs.l, regs.h);
                memory[hl] = regs.a;

                // Decrement the whole of hl before breaking it back into its parts
                hl--;
                regs.h = hl >> 8 & 0xFF;
                regs.l = hl & 0xFF;

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
            program_counter = convert_bytes_to_word(memory[program_counter + 1], memory[program_counter + 2]);
            break;
        case LOAD_A_FROM_LOW_POINTER:
            {
                uint16_t address = convert_bytes_to_word(memory[program_counter + 1], 0xFF);
                regs.a = memory[address];
            }

            program_counter += 2;
            break;
        case DISABLE_IME:
            ime = false;
            program_counter++;
            break;
        case LOAD_A_FROM_POINTER:
            regs.a = memory[convert_bytes_to_word(memory[program_counter + 1], memory[program_counter + 2])];
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
                return 1;
            }
            program_counter += 2;
            break;
        default:
            // TODO is there a way to format to hex?
            std::cerr << "Unknown opcode: 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(memory[program_counter]) << " at pc 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(program_counter) << '\n';
            return 1;
        }
    }

    return 0;
}