#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "rom.h"

bool rom_load(Arena *arena, Rom *rom, Str8 path)
{
    ArenaBackup arenaBck = arena_backup(arena);
    FILE *romFile = fopen(str8_to_cstr(arenaBck.arena, path), "rb");
    arena_restore(&arenaBck);
    if (!romFile) {
        fprintf(stderr, "Failed to open file '%.*s': %s\n", STR8_VARG(path), strerror(errno));
        fclose(romFile);
        return false;
    }

    uint8_t *romData = arena_push_zero(arena, MAX_ROM_SIZE + 1);
    int32_t romSize = (int32_t)fread(romData, sizeof(uint8_t), MAX_ROM_SIZE + 1, romFile);
    if (ferror(romFile) != 0) {
        fprintf(stderr, "Failed to read ROM file '%.*s': %s\n", STR8_VARG(path), strerror(errno));
        fclose(romFile);
        return false;
    }
    if (romSize > MAX_ROM_SIZE) {
        fprintf(stderr, "ROM file is too large '%.*s'\n", STR8_VARG(path));
        fclose(romFile);
        return false;
    }
    fclose(romFile);

    if (romSize <= INES_HEADER_SIZE) {
        fprintf(stderr, "Invalid ROM file '%.*s'\n", STR8_VARG(path));
        return false;
    }

    // iNES HEADER BYTES:
    // +----+   +----+----+----+----+----+----+----+   +----+
    // | 00 |...| 03 | 04 | 05 | 06 | 07 | 08 | 09 |...| 0F |
    // +----+   +----+----+----+----+----+----+----+   +----+
    //  \___________/  |    |    |    |    |   \___________/
    //        |        |    |    |    |    |         |
    //        |        |    |    |    |    |         reserved, all zeros
    //        |        |    |    |    |    number of 8KB PRG RAM units
    //        |        |    |    |    control byte 2
    //        |        |    |    control byte 1
    //        |        |    number of 8KB CHR ROM units
    //        |        number of 16KB PRG ROM units
    //        constant "NES^Z"
    uint8_t *header = romData;

    if (header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1A) {
        fprintf(stderr, "Not an iNES ROM file '%.*s'\n", STR8_VARG(path));
        return false;
    }

    // CONTROL BYTE 1:
    // +---+   +---+---+---+---+---+
    // | 7 |...| 4 | 3 | 2 | 1 | 0 |
    // +---+   +---+---+---+---+---+
    //  \_________/  |   |   |   |
    //       |       |   |   |   if vertical (1) or horizontal (0) mirroring
    //       |       |   |   if battery-backed RAM at $6000-$7FFF
    //       |       |   if a 512-byte trainer at $7000-$71FF
    //       |       if four-screen VRAM layout
    //       low byte of ROM mapper type
    uint8_t ctrl1 = header[6];

    // CONTROL BYTE 2:
    // +---+   +---+---+   +---+
    // | 7 |...| 4 | 3 |...| 0 |
    // +---+   +---+---+   +---+
    //  \_________/ \_________/
    //       |           |
    //       |           all zeros for iNES 1.0
    //       high byte of ROM mapper type
    uint8_t ctrl2 = header[7];

    if (ctrl2 & 0x0F) {
        fprintf(stderr, "Unsupported iNES version\n");
        return false;
    }

    rom->mapper = (uint8_t) (ctrl2 & 0xF0) | (ctrl1 >> 4);
    if (rom->mapper != NROM) {
        fprintf(stderr, "Unsupported mapper: %d\n", rom->mapper);
        return false;
    }

    rom->mirror = (ctrl1 & 0x08) ? FOUR_SCREEN : ((ctrl1 & 0x1) ? VERTICAL : HORIZONTAL);

    int32_t trainerSize = (ctrl1 & 0x04) ? 512 : 0;

    rom->prgSize = header[4] * KB(16);
    rom->chrSize = header[5] * KB(8);
    if (romSize < (INES_HEADER_SIZE + trainerSize + rom->prgSize + rom->chrSize)) {
        fprintf(stderr, "Invalid ROM file '%.*s'\n", STR8_VARG(path));
        return false;
    }

    rom->prg = header + INES_HEADER_SIZE + trainerSize;
    rom->chr = header + INES_HEADER_SIZE + trainerSize + rom->prgSize;

    return true;
}

uint8_t
rom_read(Rom *rom, uint16_t addr)
{
    uint8_t result = 0;
    switch (rom->mapper) {
        case NROM: {
            if (0x6000 <= addr && addr <= 0x7FFF) {
                // RAM
            }
            else if (0x8000 <= addr && addr <= 0xBFFF) {
                // ROM 1
                result = rom->prg[addr - 0x8000];
            }
            else {
                // ROM 2
                result = rom->prg[addr - 0xC000];
            }
        } break;
        default: {
            UNREACHABLE();
        }
    }
    return result;
}

void
rom_write(Rom *rom, uint16_t addr, uint8_t value)
{

}
