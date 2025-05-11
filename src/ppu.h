#ifndef PPU_H
#define PPU_H

#include "bus.h"

typedef struct Ppu Ppu;
struct Ppu
{
    Bus *bus;
};

#endif //PPU_H
