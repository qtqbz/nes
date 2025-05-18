#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#include "rom.h"

#define CPU_RAM_SIZE KB(2)
#define PPU_RAM_SIZE KB(2)
#define PPU_PALETTE_SIZE 32
#define PPU_OAM_SIZE 256

typedef struct Mmu Mmu;
struct Mmu
{
    // CPU Memory Mapping:
    // - $0000-$1FFF CPU RAM
    //   - $0000–$07FF WRAM
    //     - $0000-$00FF zero page
    //     - $0100-$01FF stack
    //   - $0800–$1FFF mirrors $0000–$07FF
    // - $2000-$401F IO registers
    //   - $2000–$2007 PPU registers
    //     - 0x2000 Controller
    //     - 0x2001 Mask
    //     - 0x2002 Status
    //     - 0x2003 OAM Address
    //     - 0x2004 OAM Data
    //     - 0x2005 Scroll
    //     - 0x2006 Address
    //     - 0x2007 Data
    //   - $2008–$3FFF mirrors $2000–$2007
    //   - $4000–$4017 APU+IO registers
    //     - $4000–$4003 Pulse 1
    //       - $4000 SQ1_VOL
    //       - $4001 SQ1_SWEEP
    //       - $4002 SQ1_LO
    //       - $4003 SQ1_HI
    //     - $4004–$4007 Pulse 2
    //       - $4004 SQ2_VOL
    //       - $4005 SQ2_SWEEP
    //       - $4006 SQ2_LO
    //       - $4007 SQ2_HI
    //     - $4008–$400B Triangle
    //       - $4008 TRI_LINEAR
    //       - $4009 unused
    //       - $400A TRI_LO
    //       - $400B TRI_HI
    //     - $400C–$400F Noise
    //       - $400C NOISE_VOL
    //       - $400D unused
    //       - $400E NOISE_LO
    //       - $400F NOISE_HI
    //     - $4010–$4013 DMC
    //       - $4010 DMC_FREQ
    //       - $4011 DMC_RAW
    //       - $4012 DMC_START
    //       - $4013 DMC_LEN
    //     - $4014 OAM DMA
    //     - $4015 Sound channels enable
    //     - $4016 Joystick strobe
    //     - $4017 Frame counter control
    //   - $4018–$401F APU+IO registers (ignored)
    // - $4020–$FFFF ROM
    //   - $4020–$5FFF Expansion ROM
    //   - $6000–$7FFF Save RAM
    //   - $8000–$FFFF PRG ROM
    Rom *rom;
    uint8_t cpuRam[CPU_RAM_SIZE];

    // PPU Memory Mapping:
    // - $0000-$1FFF CHR ROM
    //   - $0000-$0FFF pattern table 0
    //   - $1000-$1FFF pattern table 1
    // - $2000-$3EFF PPU RAM
    //   - $2000-$23BF name table 0
    //   - $2400-$27FF name table 1
    //   - $2800-$2BFF name table 2
    //   - $2C00-$2FFF name table 3
    //   - $3000-$3EFF unused
    // - $3F00–$3FFF ROM
    //   - $3F00-$3F1F palette RAM indexes
    //   - $3F20-$3FFF mirrors $3F00-$3F1F
    // - $4000–$FFFF mirrors $0000-$3FFF
    uint8_t ppuRam[PPU_RAM_SIZE];
    uint8_t ppuPalette[PPU_PALETTE_SIZE];
    uint8_t ppuOam[PPU_OAM_SIZE];
};

uint8_t mmu_cpu_read(Mmu *mmu, uint16_t addr);
uint16_t mmu_cpu_read16(Mmu *mmu, uint16_t addr);
void mmu_cpu_write(Mmu *mmu, uint16_t addr, uint8_t value);

uint8_t mmu_ppu_read(Mmu *mmu, uint16_t addr);
void mmu_ppu_write(Mmu *mmu, uint16_t addr, uint8_t value);

#endif //MMU_H
