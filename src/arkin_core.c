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
// Platform
//

#ifdef ARKIN_OS_LINUX

#include <unistd.h>
#include <time.h>

typedef struct _ArOsState _ArOsState;
struct _ArOsState {
    B8 inited;
    U32 page_size;
    F64 start_time;
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
}

void _ar_os_terminate(void) {
    _ar_os_state.inited = false;
}

F64 ar_os_get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec + ts.tv_nsec * 1e-9) - _ar_os_state.start_time;
}

U32 ar_os_page_size(void) {
    return _ar_os_state.page_size;
}

#endif
