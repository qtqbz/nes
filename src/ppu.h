#ifndef PPU_H
#define PPU_H

#include "mmu.h"

typedef struct Ppu Ppu;
struct Ppu
{
    Mmu *mmu;
};

#endif //PPU_H
