#ifndef NES_H
#define NES_H

#include <stdint.h>

#include "rom.h"
#include "bus.h"
#include "cpu.h"
#include "ppu.h"

#define NES_DISPLAY_WIDTH_PX 256
#define NES_DISPLAY_HEIGHT_PX 240

typedef struct Nes Nes;
struct Nes
{
    Rom *rom;
    Bus *bus;
    Cpu *cpu;
    Ppu *ppu;
};

void nes_display_update(Arena *arena, Nes *nes, uint32_t *pixels);

#endif //NES_H
