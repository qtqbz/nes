#include <string.h> // srtlen, memcpy
#include <stdio.h> // vsnprintf, vsprintf

#include "utils.h"
#include "str8.h"

bool
char_is_lower(uint8_t c)
{
    bool result = ('a' <= c) && (c <= 'z');
    return result;
}

bool
char_is_upper(uint8_t c)
{
    bool result = ('A' <= c) && (c <= 'Z');
    return result;
}

uint8_t
char_to_lower(uint8_t c)
{
  if (char_is_upper(c)) {
    c += ('a' - 'A');
  }
  return c;
}

uint8_t
char_to_upper(uint8_t c)
{
  if (char_is_lower(c)) {
    c += ('A' - 'a');
  }
  return c;
}

Str8
str8(uint8_t *bytes, int32_t length)
{
    Str8 result = {};
    result.bytes = bytes;
    result.length = length;
    return result;
}

Str8
str8_from_cstr(char *cstr)
{
    Str8 result = str8((uint8_t *)cstr, (int32_t)strlen(cstr));
    return result;
}

char *
str8_to_cstr(Arena *arena, Str8 str)
{
    char *result = (char *)arena_push(arena, str.length + 1);
    for (int32_t i = 0; i < str.length; i++) {
        result[i] = str.chars[i];
    }
    result[str.length] = '\0';
    return result;
}

bool
str8_is_equal(Str8 a, Str8 b, StrEqualsFlags flags)
{
    bool isEqual = false;

    if (a.length == b.length) {
        isEqual = true;
        bool isCaseInsensitive = flags & CaseInsensitive;
        for (int32_t i = 0; i < a.length; i++) {
            uint8_t ca = a.bytes[i];
            uint8_t cb = b.bytes[i];

            if (isCaseInsensitive) {
                ca = char_to_lower(ca);
                cb = char_to_lower(cb);
            }

            if (ca != cb) {
                isEqual = false;
                break;
            }
        }
    }
    return isEqual;
}

Str8
str8_vsprintf(Arena *arena, char *fmt, va_list args)
{
    va_list argsCpy;
    va_copy(argsCpy, args);

    int32_t count = vsnprintf(NULL, 0, fmt, args);
    ASSERT(count > 0);

    Str8 result;
    result.bytes = arena_push(arena, count);
    result.length = vsprintf((char *)result.bytes, fmt, argsCpy);
    ASSERT(result.length == count);

    va_end(argsCpy);
    return result;
}

Str8
str8_sprintf(Arena *arena, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    Str8 result = str8_vsprintf(arena, fmt, args);

    va_end(args);
    return result;
}

Str8
str8_list_first(Str8List list)
{
    return (list.head != NULL) ? list.head->str : STR8_EMPTY;
}

Str8
str8_list_last(Str8List list)
{
    return (list.tail != NULL) ? list.tail->str : STR8_EMPTY;
}

Str8Node *
str8_list_append_node(Str8List *list, Str8Node *node)
{
    node->next = NULL;
    node->prev = list->tail;
    if (list->tail != NULL) {
        list->tail->next = node;
    }
    else {
        list->head = node;
    }
    list->tail = node;
    list->count++;
    list->totalLength += node->str.length;
    return node;
}

Str8Node *
str8_list_append(Arena *arena, Str8List *list, Str8 str)
{
    Str8Node *node = (Str8Node *)arena_push_zero(arena, sizeof(Str8Node));
    node->str = str;
    return str8_list_append_node(list, node);
}

Str8Node *
str8_list_appendf(Arena *arena, Str8List *list, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    Str8 str = str8_vsprintf(arena, fmt, args);

    va_end(args);
    return str8_list_append(arena, list, str);
}

Str8Node *
str8_list_prepend_node(Str8List *list, Str8Node *node)
{
    node->prev = NULL;
    node->next = list->head;
    if (list->head != NULL) {
        list->head->prev = node;
    }
    else {
        list->tail = node;
    }
    list->head = node;
    list->count++;
    list->totalLength += node->str.length;
    return node;
}

Str8Node *
str8_list_prepend(Arena *arena, Str8List *list, Str8 str)
{
    Str8Node *node = (Str8Node *)arena_push_zero(arena, sizeof(Str8Node));
    node->str = str;
    return str8_list_prepend_node(list, node);
}

Str8Node *
str8_list_prependf(Arena *arena, Str8List *list, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    Str8 str = str8_vsprintf(arena, fmt, args);

    va_end(args);
    return str8_list_prepend(arena, list, str);
}

Str8
str8_list_join(Arena *arena, Str8List list, uint8_t delim)
{
    Str8 str = {};
    str.length = list.totalLength + list.count - 1;
    str.bytes = (uint8_t *)arena_push(arena, str.length);

    int32_t i = 0;
    Str8Node *node = list.head;
    while (node != NULL) {
        memcpy(str.bytes + i, node->str.bytes, node->str.length);
        i += node->str.length;
        node = node->next;
        if (node != NULL) {
            str.bytes[i++] = delim;
        }
    }
    return str;
}
