#include "mmu.h"

uint8_t
mmu_cpu_read(Mmu *mmu, uint16_t addr)
{
    uint8_t result = 0;
    if (addr <= 0x1FFF) {
        result = mmu->cpuRam[addr & 0x07FF];
    }
    else if (addr <= 0x3FFF) {
        result = mmu_ppu_read(mmu, addr & 0x7);
    }
    else if (addr <= 0x401F) {
        // IO registers
    }
    else {
        // ROM
    }
    return result;
}

uint16_t
mmu_cpu_read16(Mmu *mmu, uint16_t addr)
{
    uint8_t lo = mmu_cpu_read(mmu, addr);
    uint8_t hi;
    if ((addr & 0x00FF) == 0x00FF) {
        // HW bug when the page boundary is crossed
        hi = mmu_cpu_read(mmu, addr & 0xFF00);
    }
    else {
        hi = mmu_cpu_read(mmu, addr + 1);
    }
    uint16_t result = (uint16_t)((hi << 8) | lo);
    return result;
}

void
mmu_cpu_write(Mmu *mmu, uint16_t addr, uint8_t value)
{
    if (addr <= 0x1FFF) {
        mmu->cpuRam[addr & 0x07FF] = value;
    }
    else if (addr <= 0x3FFF) {
        mmu_ppu_write(mmu, addr & 0x7, value);
    }
    else if (addr <= 0x401F) {
        // IO registers
    }
    else {
        // ROM
    }
}

uint8_t
mmu_ppu_read(Mmu *mmu, uint16_t addr)
{
    return 0;
}

void
mmu_ppu_write(Mmu *mmu, uint16_t addr, uint8_t value)
{

}
