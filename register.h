#ifndef REGISTER_H
#define REGISTER_H

struct registers
{
    unsigned char a;
    // TODO should these be individual flags instead or one char?
    unsigned char f;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char h;
    unsigned char l;
};

void set_c(unsigned char &flags, bool flag);
void set_h(unsigned char &flags, bool flag);
void set_n(unsigned char &flags, bool flag);
void set_z(unsigned char &flags, bool flag);

bool get_c(unsigned char flags);
bool get_h(unsigned char flags);
bool get_n(unsigned char flags);
bool get_z(unsigned char flags);

#endif // REGISTER_H
