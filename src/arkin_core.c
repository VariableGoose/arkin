#include "arkin_core.h"

#include <stdlib.h>

_ArkinCoreState _ac = {0};

static void *_ac_default_malloc(U64 size, const char *file, U32 line) {
    (void) file;
    (void) line;
    return malloc(size);
}

static void *_ac_default_realloc(void *ptr, U64 size, const char *file, U32 line) {
    (void) file;
    (void) line;
    return realloc(ptr, size);
}

static void _ac_default_free(void *ptr, const char *file, U32 line) {
    (void) file;
    (void) line;
    return free(ptr);
}

void arkin_init(const ArkinCoreDesc *desc) {
    _ar_os_init();

    _ac.malloc  = desc->malloc  == NULL ? _ac_default_malloc  : desc->malloc;
    _ac.realloc = desc->realloc == NULL ? _ac_default_realloc : desc->realloc;
    _ac.free    = desc->free    == NULL ? _ac_default_free    : desc->free;
}

void arkin_terminate(void) {
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

ARKIN_THREAD ArArena *scratch_arenas[2] = {0};

static ArArena *get_non_conflicting_scratch_arena(ArArena **conflicting, U32 count) {
    if (count == 0) {
        return scratch_arenas[0];
    }

    for (U8 i = 0; i < count; i++) {
        for (U8 j = 0; j < arrlen(scratch_arenas); j++) {
            if (scratch_arenas[j] == conflicting[i]) {
                continue;
            }

            return scratch_arenas[j];
        }
    }

    return NULL;
}

ArTemp ar_scratch_get(ArArena **conflicting, U32 count) {
    ArArena *scratch = get_non_conflicting_scratch_arena(conflicting, count);
    return ar_temp_begin(scratch);
}

//
// Platform
//

#ifdef ARKIN_OS_LINUX

#include <unistd.h>
#include <time.h>
#include <sys/mman.h>

typedef struct _ArOsState _ArOsState;
struct _ArOsState {
    B8 inited;
    U32 page_size;
    F64 start_time;
};

typedef struct _ArOsAllocInfo _ArOsAllocInfo;
struct _ArOsAllocInfo {
    U64 size;
    U64 requested_commited;
    U64 commited;
};

static _ArOsState _ar_os_state = {0};

void _ar_os_init(void) {
    if (_ar_os_state.inited) {
        return;
    }

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    _ar_os_state = (_ArOsState) {
        .inited = true,
        .page_size = sysconf(_SC_PAGE_SIZE),
        .start_time = ts.tv_sec + ts.tv_nsec * 1e-9,
    };

    for (U32 i = 0; i < arrlen(scratch_arenas); i++) {
        scratch_arenas[i] = ar_arena_create_default();
    }
}

void _ar_os_terminate(void) {
    _ar_os_state.inited = false;

    for (U32 i = 0; i < arrlen(scratch_arenas); i++) {
        ar_arena_destroy(&scratch_arenas[i]);
    }
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

#endif
