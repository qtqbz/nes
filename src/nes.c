#include "nes.h"

bool
nes_init(Arena *arena, Nes *nes, Str8 romPath)
{
    Rom *rom = &nes->rom;
    if (!rom_load(arena, rom, romPath)) {
        return false;
    }

    Mmu *mmu = &nes->mmu;
    mmu->rom = rom;

    Cpu *cpu = &nes->cpu;
    cpu->mmu = mmu;
    if (!cpu_init(cpu)) {
        return false;
    }

    Ppu *ppu = &nes->ppu;
    ppu->mmu = mmu;

    return true;
}

void
nes_display_update(Arena *arena, Nes *nes, uint32_t *pixels)
{
    cpu_tick(&nes->cpu);
}
