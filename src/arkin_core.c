#include "arkin_core.h"

#include <stdlib.h>

_arkin_core_state_t _ac = {0};

static void *_ac_default_malloc(u64_t size, const char *file, u32_t line) {
    (void) file;
    (void) line;
    return malloc(size);
}

static void *_ac_default_realloc(void *ptr, u64_t size, const char *file, u32_t line) {
    (void) file;
    (void) line;
    return realloc(ptr, size);
}

static void _ac_default_free(void *ptr, const char *file, u32_t line) {
    (void) file;
    (void) line;
    return free(ptr);
}

void arkin_init(const arkin_core_desc_t *desc) {
    _ac.malloc  = desc->malloc  == NULL ? _ac_default_malloc  : desc->malloc;
    _ac.realloc = desc->realloc == NULL ? _ac_default_realloc : desc->realloc;
    _ac.free    = desc->free    == NULL ? _ac_default_free    : desc->free;
}

void arkin_terminate(void) {
}
