#include <cstdint>
#include <iomanip>
#include <iostream>

#include "opcode.h"

#define HEX( x ) "setw(2) << setfill('0') << hex << (int)( x )"

int main()
{
    unsigned char mem[1000];
    FILE *file;
    errno_t error = fopen_s(&file, "C:\\Users\\tgrieger\\Source\\repos\\gbemu\\artifacts\\dmg_boot.bin", "rb");
    if (error)
    {
        std::cout << error << std::endl;
    }

    // Read all contents of the file into the memory buffer
    int pos = 0;
    while (fread(&mem[pos++], 1, 1, file))
    {
        pos++;
    }

    fclose(file);

    uint16_t program_counter {};
    uint16_t stack_pointer {};
    while (true)
    {
        switch (mem[program_counter])
        {
        case NO_OP:
           program_counter++;
           break;
        case LOAD_SP_FROM_MEMORY:
            stack_pointer = static_cast<uint16_t>(mem[program_counter + 1]) | static_cast<uint16_t>(mem[program_counter + 2]) << 8;
            program_counter += 3;
            break;
        default:
            std::cout << "Unknown opcode: 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(mem[program_counter]) << " at pc 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(program_counter) << '\n';
            return 1;
        }
    }

    return 0;
}