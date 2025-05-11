#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>

typedef struct Arena Arena;
struct Arena
{
    uint8_t *buf;
    int32_t cap;
    int32_t pos;
};

typedef struct ArenaBackup ArenaBackup;
struct ArenaBackup
{
    Arena *arena;
    int32_t pos;
};

Arena arena_make(uint8_t *buf, int32_t cap);
void arena_clear(Arena *arena);

void *arena_push(Arena *arena, int32_t count);
void *arena_push_zero(Arena *arena, int32_t count);
void arena_pop(Arena *arena, int32_t count);

ArenaBackup arena_backup(Arena *arena);
void arena_restore(ArenaBackup *bck);

#endif //ARENA_H
