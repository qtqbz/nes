#ifndef STR8_H
#define STR8_H

#include <stdint.h>
#include <stdarg.h>

#include "arena.h"

#define STR8_LITERAL(s) (Str8){.chars = (s), .length = (sizeof(s) - 1)}
#define STR8_EMPTY STR8_LITERAL("")
#define STR8_VARG(s) (s).length, ((s).chars)

typedef struct Str8 Str8;
struct Str8
{
    union
    {
        uint8_t *bytes;
        char *chars; // not null-terminated!
    };
    int32_t length;
};

typedef struct Str8Node Str8Node;
struct Str8Node
{
    Str8 str;
    Str8Node *next;
    Str8Node *prev;
};

typedef struct Str8List Str8List;
struct Str8List
{
    Str8Node *head;
    Str8Node *tail;
    int32_t count;
    int32_t totalLength;
};

typedef int32_t StrEqualsFlags;
enum StrEqualsFlags
{
    CaseInsensitive = (1 << 0),
};

bool char_is_lower(uint8_t c);
bool char_is_upper(uint8_t c);

uint8_t char_to_lower(uint8_t c);
uint8_t char_to_upper(uint8_t c);

Str8 str8(uint8_t *bytes, int32_t length);
Str8 str8_from_cstr(char *cstr);
char *str8_to_cstr(Arena *arena, Str8 s);

bool str8_is_equal(Str8 a, Str8 b, StrEqualsFlags flags);

Str8 str8_vsprintf(Arena *arena, char *fmt, va_list args);
Str8 str8_sprintf(Arena *arena, char *fmt, ...);

Str8 str8_list_first(Str8List list);
Str8 str8_list_last(Str8List list);
Str8Node *str8_list_append(Arena *arena, Str8List *list, Str8 str);
Str8Node *str8_list_appendf(Arena *arena, Str8List *list, char *fmt, ...);
Str8Node *str8_list_append_node(Str8List *list, Str8Node *node);
Str8Node *str8_list_prepend(Arena *arena, Str8List *list, Str8 str);
Str8Node *str8_list_prependf(Arena *arena, Str8List *list, char *fmt, ...);
Str8Node *str8_list_prepend_node(Str8List *list, Str8Node *node);
Str8 str8_list_join(Arena *arena, Str8List list, uint8_t delim);

#endif //STR8_H
