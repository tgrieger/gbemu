#include "register.h"

void set_flag(unsigned char &flags, const bool flag, const int bit)
{
    // General formula for setting the nth bit to x: `number = number & ~(1 << n) | (x << n)`
    flags = flags & ~(1 << bit) | flag << bit;
}

bool get_flag(const unsigned char flags, const int bit)
{
    return (flags & 1 << bit) >> bit;
}

void set_c(unsigned char &flags, const bool flag)
{
    set_flag(flags, flag, 4);
}

void set_h(unsigned char &flags, const bool flag)
{
    set_flag(flags, flag, 5);
}

void set_n(unsigned char &flags, const bool flag)
{
    set_flag(flags, flag, 6);
}

void set_z(unsigned char &flags, const bool flag)
{
    set_flag(flags, flag, 7);
}

bool get_c(unsigned char flags)
{
    return get_flag(flags, 4);
}

bool get_h(unsigned char flags)
{
    return get_flag(flags, 5);
}

bool get_n(unsigned char flags)
{
    return get_flag(flags, 6);
}

bool get_z(unsigned char flags)
{
    return get_flag(flags, 7);
}
