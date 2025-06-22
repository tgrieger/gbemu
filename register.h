#ifndef REGISTER_H
#define REGISTER_H

struct registers
{
    unsigned char a;
    // TODO should these be individual flags instead of one char?
    unsigned char f;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char h;
    unsigned char l;
};

#endif // REGISTER_H
