#include "arkin_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct _ArkinCoreState _ArkinCoreState;
struct _ArkinCoreState {
    ArThreadCtx *thread_ctx;
};
static _ArkinCoreState _ar_core = {0};

void arkin_init(const ArkinCoreDesc *desc) {
    ArkinCoreDesc _desc = *desc;
    if (desc->thread_pool_capacity == 0) {
        _desc.thread_pool_capacity = 256;
    }

    _ar_os_init(_desc.thread_pool_capacity);

    _ar_core.thread_ctx = ar_thread_ctx_create();
    ar_thread_ctx_set(_ar_core.thread_ctx);
}

void arkin_terminate(void) {
    ar_thread_ctx_set(NULL);
    ar_thread_ctx_destroy(&_ar_core.thread_ctx);

    _ar_os_terminate();
}

//
// Arena
//

static U64 align_to_page_size(U64 value) {
    U64 page_size = ar_os_page_size();
    return value + (page_size - value) % page_size;
}

struct ArArena {
    U64 capacity;
    U64 commited;
    U64 position;
    U8 *ptr;
};

ArArena *ar_arena_create(U64 capacity) {
    ArArena *arena = ar_os_mem_reserve(capacity + sizeof(ArArena));
    ar_os_mem_commit(arena, ar_os_page_size() + sizeof(ArArena));
    *arena = (ArArena) {
        .capacity = capacity,
        .commited = ar_os_page_size(),
        .position = 0,
        .ptr = (U8 *) &arena[1],
    };

    return arena;
}

ArArena *ar_arena_create_default(void) {
    return ar_arena_create(4 * 1llu << 30);
}

void ar_arena_destroy(ArArena **arena) {
    ar_os_mem_release(*arena);
    *arena = NULL;
}

void *ar_arena_push(ArArena *arena, U64 size) {
    void *result = ar_arena_push_no_zero(arena, size);
    memset(result, 0, size);
    return result;
}

void *ar_arena_push_no_zero(ArArena *arena, U64 size) {
    void *result = arena->ptr + arena->position;

    arena->position += size;

    U64 aligned = align_to_page_size(arena->position);
    if (aligned > arena->commited) {
        ar_os_mem_commit(arena, aligned);
        arena->commited = aligned;
    }

    return result;
}

void ar_arena_pop(ArArena *arena, U64 size) {
    if (size > arena->position) {
        size = arena->position;
    }

    arena->position -= size;

    U64 aligned = align_to_page_size(arena->position);
    if (aligned < arena->commited && aligned != 0) {
        ar_os_mem_decommit(arena, arena->commited - aligned);
        arena->commited = aligned;
    }
}

void ar_arena_reset(ArArena *arena) {
    ar_arena_pop(arena, arena->position);
}

U64 ar_arena_used(const ArArena *arena) {
    return arena->position;
}

ArTemp ar_temp_begin(ArArena *arena) {
    return (ArTemp) {
        .arena = arena,
        .pos = arena->position,
    };
}

void ar_temp_end(ArTemp *temp) {
    U64 diff = temp->arena->position - temp->pos;
    ar_arena_pop(temp->arena, diff);

    // I'm sorry for this.
    *(ArArena **) &temp->arena = NULL;
    *(U32 *) &temp->pos = 0;
}

//
// Thread context
//

#define SCRATCH_ARENA_COUNT 2

struct ArThreadCtx {
    ArArena *scratch_arenas[SCRATCH_ARENA_COUNT];
};

ARKIN_THREAD ArThreadCtx *_ar_thread_ctx_curr = NULL;

ArThreadCtx *ar_thread_ctx_create(void) {
    ArArena *arenas[SCRATCH_ARENA_COUNT] = {0};

    for (U32 i = 0; i < ar_arrlen(arenas); i++) {
        arenas[i] = ar_arena_create_default();
    }

    ArThreadCtx *ctx = ar_arena_push_type(arenas[0], ArThreadCtx);
    for (U32 i = 0; i < ar_arrlen(arenas); i++) {
        ctx->scratch_arenas[i] = arenas[i];
    }

    return ctx;
}

void ar_thread_ctx_destroy(ArThreadCtx **ctx) {
    // Copy arena pointers because ctx lives on the first one so if we free the
    // first and then try to access the second one the program will segfault.
    ArArena *arenas[SCRATCH_ARENA_COUNT] = {0};
    for (U32 i = 0; i < ar_arrlen((*ctx)->scratch_arenas); i++) {
        arenas[i] = (*ctx)->scratch_arenas[i];
    }

    for (U32 i = 0; i < ar_arrlen(arenas); i++) {
        ar_arena_destroy(&arenas[i]);
    }

    *ctx = NULL;
}

void ar_thread_ctx_set(ArThreadCtx *ctx) {
    _ar_thread_ctx_curr = ctx;
}

static ArArena *get_non_conflicting_scratch_arena(ArArena **conflicting, U32 count) {
    if (_ar_thread_ctx_curr == NULL) {
        return NULL;
    }

    if (count == 0) {
        return _ar_thread_ctx_curr->scratch_arenas[0];
    }

    for (U8 i = 0; i < count; i++) {
        for (U8 j = 0; j < ar_arrlen(_ar_thread_ctx_curr->scratch_arenas); j++) {
            if (_ar_thread_ctx_curr->scratch_arenas[j] == conflicting[i]) {
                continue;
            }

            return _ar_thread_ctx_curr->scratch_arenas[j];
        }
    }

    return NULL;
}

ArTemp ar_scratch_get(ArArena **conflicting, U32 count) {
    ArArena *scratch = get_non_conflicting_scratch_arena(conflicting, count);
    if (scratch == NULL) {
        return (ArTemp) {0};
    }
    return ar_temp_begin(scratch);
}

//
// Strings
//

// Char helpers
B8 ar_char_is_numeric(char c) {
    return c >= '0' && c <= '9';
}

B8 ar_char_is_alpha(char c) {
    return ar_char_is_lower(c) || ar_char_is_upper(c);
}

B8 ar_char_is_lower(char c) {
    return c >= 'a' && c <= 'z';
}

B8 ar_char_is_upper(char c) {
    return c >= 'A' && c <= 'Z';
}

B8 ar_char_is_whitespace(char c) {
    return c == ' ' ||
        c == '\t' ||
        c == '\n' ||
        c == '\r' ||
        c == '\v' ||
        c == '\f';
}

char ar_char_to_lower(char c) {
    if (ar_char_is_upper(c)) {
        c += 'a' - 'A';
    }
    return c;
}

char ar_char_to_upper(char c) {
    if (ar_char_is_lower(c)) {
        c -= 'a' - 'A';
    }
    return c;
}

// String list

void ar_str_list_push(ArArena *arena, ArStrList *list, ArStr str) {
    ArStrListNode *node = ar_arena_push_type(arena, ArStrListNode);
    node->str = str;

    ar_dll_push_back(list->first, list->last, node);
}

void ar_str_list_push_front(ArArena *arena, ArStrList *list, ArStr str) {
    ArStrListNode *node = ar_arena_push_type(arena, ArStrListNode);
    node->str = str;

    ar_dll_push_front(list->first, list->last, node);
}

void ar_str_list_pop(ArStrList *list) {
    ar_dll_pop_back(list->first, list->last);
}

void ar_str_list_pop_front(ArStrList *list) {
    ar_dll_pop_front(list->first, list->last);
}

ArStr ar_str_list_join(ArArena *arena, ArStrList list) {
    U64 len = 0;
    for (ArStrListNode *curr = list.first; curr != NULL; curr = curr->next) {
        len += curr->str.len;
    }

    U8 *data = ar_arena_push_no_zero(arena, len);
    U64 i = 0;
    for (ArStrListNode *curr = list.first; curr != NULL; curr = curr->next) {
        for (U64 j = 0; j < curr->str.len; j++) {
            data[i + j] = curr->str.data[j];
        }
        i += curr->str.len;
    }

    return ar_str(data, len);
}

ArStr ar_str_format(ArArena *arena, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    va_list args_len;
    va_copy(args_len, args);

    U64 len = vsnprintf(NULL, 0, fmt, args_len);
    va_end(args_len);
    // Temporary length for null-terminator.
    len++;

    U8 *data = ar_arena_push_arr_no_zero(arena, U8, len);

    vsnprintf((char *) data, len, fmt, args);
    va_end(args);

    // Remove null-terminator from string.
    len--;
    return ar_str(data, len);
}

ArStr ar_str_push_copy(ArArena *arena, ArStr str) {
    U8 *data = ar_arena_push_arr_no_zero(arena, U8, str.len);
    memcpy(data, str.data, str.len);
    return ar_str(data, str.len);
}

B8 ar_str_match(ArStr a, ArStr b, ArStrMatchFlag flags) {
    if (a.len != b.len && !(flags & AR_STR_MATCH_FLAG_SLOPPY_LENGTH)) {
        return false;
    }
    U64 len = ar_min(a.len, b.len);
    for (U64 i = 0; i < len; i++) {
        char _a = a.data[i];
        char _b = b.data[i];
        if (flags & AR_STR_MATCH_FLAG_CASE_INSENSITIVE) {
            _a = ar_char_to_lower(_a);
            _b = ar_char_to_lower(_b);
        }

        if (_a != _b) {
            return false;
        }
    }

    return true;
}

ArStr ar_str_sub(ArStr str, U64 start, U64 end) {
    if (start > end) {
        U64 temp = start;
        start = end;
        end = temp;
    }

    if (start >= str.len) {
        start = str.len - 1;
    }
    if (end >= str.len) {
        start = str.len - 1;
    }

    return ar_str(str.data + start, end - start + 1);
}

ArStr ar_str_sub_len(ArStr str, U64 start, U64 len) {
    return ar_str_sub(str, start, start + len - 1);
}

U64 ar_str_find(ArStr haystack, ArStr needle, ArStrMatchFlag flags) {
    U64 index = 0;
    for (U64 i = 0; i < haystack.len; i++) {
        if (flags & AR_STR_MATCH_FLAG_LAST) {
            index = haystack.len - i - 1;
        } else {
            index = i;
        }

        ArStr sub = ar_str_sub_len(haystack, index, needle.len);
        if (ar_str_match(needle, sub, flags | AR_STR_MATCH_FLAG_SLOPPY_LENGTH)) {
            return index;
        }
    }

    return haystack.len;
}

U64 ar_str_find_char(ArStr haystack, char needle, ArStrMatchFlag flags) {
    U64 index = 0;
    for (U64 i = 0; i < haystack.len; i++) {
        if (flags & AR_STR_MATCH_FLAG_LAST) {
            index = haystack.len - i - 1;
        } else {
            index = i;
        }

        char a = needle;
        char b = haystack.data[index];
        if (flags & AR_STR_MATCH_FLAG_CASE_INSENSITIVE) {
            a = ar_char_to_lower(a);
            b = ar_char_to_lower(b);
        }

        if (a == b) {
            return index;
        }
    }

    return haystack.len;
}

ArStr ar_str_trim(ArStr str) {
    ArStr front = ar_str_trim_front(str);
    ArStr back = ar_str_trim_back(front);
    return back;
}

ArStr ar_str_trim_front(ArStr str) {
    U64 i = 0;
    for (; i < str.len && ar_char_is_whitespace(str.data[i]); i++);
    return ar_str_chop_start(str, i);
}

ArStr ar_str_trim_back(ArStr str) {
    U64 i = 0;
    for (; i < str.len && ar_char_is_whitespace(str.data[str.len - i - 1]); i++);
    return ar_str_chop_end(str, i);
}

ArStrList ar_str_split(ArArena *arena, ArStr str, ArStr delim, ArStrMatchFlag flags) {
    ArStrList list = {0};

    U64 start = 0;
    while (start < str.len) {
        U64 next = start + ar_str_find(ar_str_chop_start(str, start), delim, flags);

        if (next == start) {
            ar_str_list_push(arena, &list, ar_str(str.data, 0));
        } else {
            ar_str_list_push(arena, &list, ar_str_sub(str, start, next - 1));
        }

        start = next + delim.len;
    }

    return list;
}

ArStrList ar_str_split_char(ArArena *arena, ArStr str, char delim, ArStrMatchFlag flags) {
    ArStrList list = {0};

    U64 start = 0;
    while (start < str.len) {
        U64 next = start + ar_str_find_char(ar_str_chop_start(str, start), delim, flags);

        if (next == start) {
            ar_str_list_push(arena, &list, ar_str(str.data, 0));
        } else {
            ar_str_list_push(arena, &list, ar_str_sub(str, start, next - 1));
        }

        start = next + 1;
    }

    return list;
}

//
// Hash map
//

B8 ar_memeq(const void *a, const void *b, U64 len) {
    return memcmp(a, b, len) == 0;
}

U64 ar_fvn1a_hash(const void *data, U64 len) {
    const U8 *_data = data;
    U64 hash = 2166136261u;
    for (U32 i = 0; i < len; i++) {
        hash ^= *(_data + i);
        hash *= 16777619;
    }
    return hash;
}

typedef struct ArHashMapNode ArHashMapNode;
struct ArHashMapNode {
    ArHashMapNode *next;
    ArHashMapNode *prev;

    void *key;
    void *value;
};

typedef struct ArHashMapBucket ArHashMapBucket;
struct ArHashMapBucket {
    ArHashMapNode *first;
    ArHashMapNode *last;
};

struct ArHashMap {
    ArHashMapDesc desc;

    ArHashMapBucket *buckets;
    ArHashMapNode *free_list;
    void *null_value;
};

ArHashMap *ar_hash_map_init(ArHashMapDesc desc) {
    ArHashMap *map = ar_arena_push_type_no_zero(desc.arena, ArHashMap);
    *map = (ArHashMap) {
        .desc = desc,
        .buckets = ar_arena_push_arr(desc.arena, ArHashMapBucket, desc.capacity),
        .null_value = ar_arena_push_no_zero(desc.arena, desc.value_size),
    };
    memcpy(map->null_value, desc.null_value, desc.value_size);

    return map;
}

static ArHashMapNode *hash_map_get_node(ArHashMap *map, const void *key, const void *value) {
    ArHashMapNode *node = NULL;
    if (map->free_list != NULL) {
        node = map->free_list;
        ar_sll_stack_pop(map->free_list);
    }

    if (node == NULL) {
        node = ar_arena_push_type(map->desc.arena, ArHashMapNode);
        node->key = ar_arena_push_no_zero(map->desc.arena, map->desc.key_size);
        node->value = ar_arena_push_no_zero(map->desc.arena, map->desc.value_size);
    }

    memcpy(node->key, key, map->desc.key_size);
    memcpy(node->value, value, map->desc.value_size);

    return node;
}

static ArHashMapBucket *hash_map_get_bucket(const ArHashMap *map, const void *key) {
    U64 hash = map->desc.hash_func(key, map->desc.key_size);
    U32 index = hash % map->desc.capacity;
    return &map->buckets[index];
}

B8 _ar_hash_map_insert(ArHashMap *map, const void *key, const void *value) {
    ArHashMapBucket *bucket = hash_map_get_bucket(map, key);

    if (bucket->first != NULL) {
        return false;
    }

    ArHashMapNode *node = hash_map_get_node(map, key, value);
    ar_dll_push_back(bucket->first, bucket->last, node);

    return true;
}

B8 _ar_hash_map_set(ArHashMap *map, const void *key, const void *value) {
    ArHashMapBucket *bucket = hash_map_get_bucket(map, key);

    for (ArHashMapNode *node = bucket->first; node != NULL; node = node->next) {
        if (map->desc.eq_func(key, node->key, map->desc.key_size)) {
            memcpy(node->value, value, map->desc.value_size);
            return false;
        }
    }

    ArHashMapNode *node = hash_map_get_node(map, key, value);
    ar_dll_push_back(bucket->first, bucket->last, node);

    return true;
}

B8 _ar_hash_map_remove(ArHashMap *map, const void *key) {
    ArHashMapBucket *bucket = hash_map_get_bucket(map, key);

    for (ArHashMapNode *node = bucket->first; node != NULL; node = node->next) {
        if (map->desc.eq_func(key, node->key, map->desc.key_size)) {
            ar_sll_stack_push(map->free_list, node);
            ar_dll_remove(bucket->first, bucket->last, node);

            return true;
        }
    }

    return false;
}

B8 _ar_hash_map_has(const ArHashMap *map, const void *key) {
    ArHashMapBucket *bucket = hash_map_get_bucket(map, key);

    for (ArHashMapNode *node = bucket->first; node != NULL; node = node->next) {
        if (map->desc.eq_func(key, node->key, map->desc.key_size)) {
            return true;
        }
    }

    return false;
}

void _ar_hash_map_get(const ArHashMap *map, const void *key, void *output) {
    ArHashMapBucket *bucket = hash_map_get_bucket(map, key);

    for (ArHashMapNode *node = bucket->first; node != NULL; node = node->next) {
        if (map->desc.eq_func(key, node->key, map->desc.key_size)) {
            memcpy(output, node->value, map->desc.value_size);
            return;
        }
    }

    memcpy(output, map->null_value, map->desc.value_size);
}

void *_ar_hash_map_get_ptr(const ArHashMap *map, const void *key) {
    ArHashMapBucket *bucket = hash_map_get_bucket(map, key);

    for (ArHashMapNode *node = bucket->first; node != NULL; node = node->next) {
        if (map->desc.eq_func(key, node->key, map->desc.key_size)) {
            return node->value;
        }
    }

    return NULL;
}

ArArena *ar_hash_map_get_arena(const ArHashMap *map) {
    return map->desc.arena;
}

//
// Pool allocator
//

// [64-bit handle]: [32-bit generation][32-bit index]

typedef struct ArPoolFreeObject ArPoolFreeObject;
struct ArPoolFreeObject {
    ArPoolFreeObject *next;
    U32 index;
};

struct ArPool {
    void *pool;
    U32 *generation;
    B8 *used;
    U32 *free_list;
    U32 free_list_next;
    U32 next_index;
    U64 object_size;
    U32 capacity;
};

ArPool *ar_pool_init(ArArena *arena, U32 capacity, U64 object_size) {
    ArPool *pool = ar_arena_push_type_no_zero(arena, ArPool);
    *pool = (ArPool) {
        .pool = ar_arena_push_no_zero(arena, object_size * capacity),
        .generation = ar_arena_push_arr(arena, U32, capacity),
        .used = ar_arena_push_arr(arena, B8, capacity),
        .free_list = ar_arena_push_arr(arena, U32, capacity),
        .object_size = object_size,
        .capacity = capacity,
    };

    return pool;
}

ArPoolHandle ar_pool_handle_create(ArPool *pool) {
    // Out of space.
    if (pool->free_list_next == 0 && pool->next_index == pool->capacity) {
        return AR_POOL_HANDLE_INVALID;
    }

    U32 index = pool->next_index;
    if (pool->free_list_next != 0) {
        index = pool->free_list[pool->free_list_next - 1];
        pool->free_list_next--;
    } else {
        pool->next_index++;
    }

    pool->used[index] = true;
    U32 generation = pool->generation[index];
    memset(pool->pool + index * pool->object_size, 0, pool->object_size);

    return (ArPoolHandle) {
        .handle = ((U64) generation) << 32 | index,
    };
}

void ar_pool_handle_destroy(ArPool *pool, ArPoolHandle handle) {
    if (!ar_pool_handle_valid(pool, handle)) {
        return;
    }

    U32 generation = (U32) (handle.handle >> 32);
    U32 index = (U32) handle.handle;

    pool->used[index] = false;
    pool->generation[index]++;
    pool->free_list[pool->free_list_next] = index;
    pool->free_list_next++;
}

B8 ar_pool_handle_valid(const ArPool *pool, ArPoolHandle handle) {
    U32 generation = (U32) (handle.handle >> 32);
    U32 index = (U32) handle.handle;

    return handle.handle != AR_POOL_HANDLE_INVALID.handle &&
        index < pool->next_index &&
        generation == pool->generation[index] &&
        pool->used[index];
}

void *ar_pool_handle_to_ptr(const ArPool *pool, ArPoolHandle handle) {
    if (!ar_pool_handle_valid(pool, handle)) {
        return NULL;
    }

    U32 index = (U32) handle.handle;
    return pool->pool + index * pool->object_size;
}

//
// Platform
//

#ifdef ARKIN_OS_LINUX

#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <pthread.h>

typedef struct _ArOsState _ArOsState;
struct _ArOsState {
    F64 start_time;
    U32 page_size;

    ArArena *os_arena;
    ArPool *thread_pool;
};

typedef struct _ArOsAllocInfo _ArOsAllocInfo;
struct _ArOsAllocInfo {
    U64 size;
    U64 requested_commited;
    U64 commited;
};

typedef struct _ArOsThread _ArOsThread;
struct _ArOsThread {
    pthread_t thread_id;
    ArThreadFunc func;
    void *args;
};

static _ArOsState _ar_os_state = {0};

void _ar_os_init(U32 thread_pool_capacity) {
    _ar_os_state = (_ArOsState) {0};

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    _ar_os_state.start_time = ts.tv_sec + ts.tv_nsec * 1e-9,

    _ar_os_state.page_size = sysconf(_SC_PAGE_SIZE);

    ArArena *arena = ar_arena_create_default();
    _ar_os_state.os_arena = arena;

    _ar_os_state.thread_pool = ar_pool_init(arena, thread_pool_capacity, sizeof(_ArOsThread));
}

void _ar_os_terminate(void) {
    ar_arena_destroy(&_ar_os_state.os_arena);
}

F64 ar_os_get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec + ts.tv_nsec * 1e-9) - _ar_os_state.start_time;
}

U32 ar_os_page_size(void) {
    return _ar_os_state.page_size;
}

void *ar_os_mem_reserve(U64 size) {
    U32 page_size = ar_os_page_size();
    size = align_to_page_size(size + sizeof(_ArOsAllocInfo));

    _ArOsAllocInfo *info = mmap(NULL, size + sizeof(_ArOsAllocInfo), PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    madvise((U8 *) info + page_size, size - page_size, MADV_DONTNEED);
    mprotect(info, page_size, PROT_READ | PROT_WRITE);
    info->size = size;
    info->commited = page_size;
    info->requested_commited = sizeof(_ArOsAllocInfo);

    return &info[1];
}

void ar_os_mem_commit(void *ptr, U64 size) {
    _ArOsAllocInfo *info = &((_ArOsAllocInfo *) ptr)[-1];
    info->requested_commited += size;
    U64 requested = align_to_page_size(info->requested_commited);
    if (requested > info->commited) {
        info->commited = requested;
        mprotect(info, info->commited, PROT_READ | PROT_WRITE);
    }
}

void ar_os_mem_decommit(void *ptr, U64 size) {
    _ArOsAllocInfo *info = &((_ArOsAllocInfo *) ptr)[-1];

    if (size > info->requested_commited) {
        size = info->requested_commited;
    }

    info->requested_commited -= size;
    if (info->requested_commited < sizeof(_ArOsAllocInfo)) {
        info->requested_commited = sizeof(_ArOsAllocInfo);
    }

    U64 requested = align_to_page_size(info->requested_commited);
    if (requested < info->commited) {
        mprotect((U8 *) ptr + requested, info->commited - requested, PROT_NONE);
        info->commited = requested;
    }
}

void ar_os_mem_release(void *ptr) {
    _ArOsAllocInfo *info = &((_ArOsAllocInfo *) ptr)[-1];

    munmap(info, info->size);
}

// TODO: Remove malloc and free call by having the OS layer handle thread info in a pool.
static void *_ar_os_thread_entry(void *args) {
    _ArOsThread *thread = args;

    ArThreadCtx *thread_ctx = ar_thread_ctx_create();
    ar_thread_ctx_set(thread_ctx);
    thread->func(thread->args);
    ar_thread_ctx_destroy(&thread_ctx);

    return NULL;
}

static void *_ar_os_thread_entry_no_ctx(void *args) {
    _ArOsThread *thread = args;

    thread->func(thread->args);

    return NULL;
}

ArThread ar_thread_create(ArThreadFunc func, void *args) {
    ArThread handle = {
        .handle = ar_pool_handle_create(_ar_os_state.thread_pool),
    };

    _ArOsThread *thread = ar_pool_handle_to_ptr(_ar_os_state.thread_pool, handle.handle);
    if (thread == NULL) {
        return handle;
    }

    thread->func = func;
    thread->args = args;

    pthread_create(&thread->thread_id, NULL, _ar_os_thread_entry, thread);

    return handle;
}

ArThread ar_thread_create_no_ctx(ArThreadFunc func, void *args) {
    ArThread handle = {
        .handle = ar_pool_handle_create(_ar_os_state.thread_pool),
    };

    _ArOsThread *thread = ar_pool_handle_to_ptr(_ar_os_state.thread_pool, handle.handle);
    thread->func = func;
    thread->args = args;

    pthread_create(&thread->thread_id, NULL, _ar_os_thread_entry_no_ctx, thread);

    return handle;
}

void ar_thread_join(ArThread thread) {
    _ArOsThread *os_thread = ar_pool_handle_to_ptr(_ar_os_state.thread_pool, thread.handle);
    if (os_thread == NULL) {
        return;
    }

    pthread_join(os_thread->thread_id, NULL);

    ar_thread_destroy(thread);
}

void ar_thread_detatch(ArThread thread) {
    _ArOsThread *os_thread = ar_pool_handle_to_ptr(_ar_os_state.thread_pool, thread.handle);
    if (os_thread == NULL) {
        return;
    }

    pthread_detach(os_thread->thread_id);

    ar_thread_destroy(thread);
}

void ar_thread_destroy(ArThread thread) {
    ar_pool_handle_destroy(_ar_os_state.thread_pool, thread.handle);
}

#endif
