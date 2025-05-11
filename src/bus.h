#ifndef BUS_H
#define BUS_H

#include <stdint.h>

#include "rom.h"

#define CPU_RAM_SIZE KB(2)
#define PPU_RAM_SIZE KB(2)
#define PPU_PALETTE_SIZE 32
#define PPU_OAM_SIZE 256

typedef struct Bus Bus;
struct Bus
{
    Rom *rom;
    uint8_t cpuRam[CPU_RAM_SIZE];
    uint8_t ppuRam[PPU_RAM_SIZE];
    uint8_t ppuPalette[PPU_PALETTE_SIZE];
    uint8_t ppuOam[PPU_OAM_SIZE];
};

#endif //BUS_H
