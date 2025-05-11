#ifndef ROM_H
#define ROM_H

#include <stdint.h>

#include "utils.h"
#include "arena.h"
#include "str8.h"

#define MAX_ROM_SIZE MB(1)
#define INES_HEADER_SIZE 16

typedef int32_t Mapper;
enum Mapper
{
    NROM = 0,
};

typedef int32_t Mirror;
enum Mirror
{
    HORIZONTAL,
    VERTICAL,
    FOUR_SCREEN,
};

typedef struct Rom Rom;
struct Rom
{
    uint8_t *prg;
    int32_t prgSize;

    uint8_t *chr;
    int32_t chrSize;

    Mirror mirror;
    Mapper mapper;
};

bool rom_load(Arena *arena, Rom *rom, Str8 path);

#endif //ROM_H
