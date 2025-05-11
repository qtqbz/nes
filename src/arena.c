#include <string.h> // memset

#include "utils.h"
#include "arena.h"

Arena
arena_make(uint8_t *buf, int32_t cap)
{
    ASSERT(buf != NULL);
    ASSERT(cap > 0);

    Arena arena = {};
    arena.buf = buf;
    arena.cap = cap;

    return arena;
}

void *
arena_push(Arena *arena, int32_t count)
{
    ASSERT(count > 0);
    ASSERT(arena->pos + count <= arena->cap);

    uint8_t *beg = arena->buf + arena->pos;
    arena->pos += count;

    return beg;
}

void *
arena_push_zero(Arena *arena, int32_t count)
{
    ASSERT(count > 0);

    uint8_t *beg = arena_push(arena, count);
    beg = memset(beg, 0, count);

    return beg;
}

void
arena_pop(Arena *arena, int32_t count)
{
    ASSERT(count > 0);
    ASSERT((arena->pos - count) >= 0);

    arena->pos -= count;
}

void
arena_clear(Arena *arena)
{
    arena->pos = 0;
}

ArenaBackup
arena_backup(Arena *arena)
{
    ArenaBackup result = {};
    result.arena = arena;
    result.pos = arena->pos;
    return result;
}

void
arena_restore(ArenaBackup *bck)
{
    bck->arena->pos = bck->pos;
}
