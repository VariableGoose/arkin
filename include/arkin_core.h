#ifndef ARKIN_CORE_H
#define ARKIN_CORE_H

#include <string.h>
#include <stdarg.h>

#ifdef __linux__
#define ARKIN_OS_LINUX
#endif

#ifdef _WIN32
#define ARKIN_OS_WINDOWS
#endif

#ifdef __clang__
#define ARKIN_COMPILER_CLANG 1
#endif

#ifdef __GNUC__
#define ARKIN_COMPILER_GCC 1
#endif

#ifdef _MSC_VER
#define ARKIN_COMPILER_MSVC 1
#endif

#if defined(ARKIN_DYNAMIC) && defined(ARKIN_OS_WINDOWS)
    // Using compiled windows DLL.
    #define ARKIN_API __declspec(dllimport)
#elif defined(ARKIN_DYNAMIC) && defined(ARKIN_COMPILE) && defined(ARKIN_OS_WINDOWS)
    // Compiling windows DLL.
    #define ARKIN_API __declspec(dllexport)
#elif defined(ARKIN_DYNAMIC) && defined(ARKIN_COMPILE) && defined(ARKIN_OS_LINUX)
    // Compiling linux shared object.
    #define ARKIN_API __attribute__((visibility("default")))
#else
    #define ARKIN_API extern
#endif

#define ARKIN_INLINE static inline

#ifdef ARKIN_OS_LINUX
#define ARKIN_THREAD __thread
#endif

#ifdef ARKIN_OS_WINDOWS
#define ARKIN_THREAD __declspec(thread)
#endif

#ifdef ARKIN_OS_LINUX
#define AR_FORMAT_FUNCTION(FORMAT_INDEX, VA_INDEX) __attribute__((format(printf, FORMAT_INDEX, VA_INDEX)))
#else
#define RE_FORMAT_FUNCTION(FORMAT_INDEX, VA_INDEX)
#endif

// https://github.com/google/sanitizers/wiki/AddressSanitizerManualPoisoning
#if defined(ARKIN_SANITIZE_ADDRESSES) && __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#include <sanitizer/asan_interface.h>
#define AR_ASAN_POISON_MEMORY_REGION(addr, size) \
  __asan_poison_memory_region((addr), (size))
#define AR_ASAN_UNPOISON_MEMORY_REGION(addr, size) \
  __asan_unpoison_memory_region((addr), (size))
#else
#define AR_ASAN_POISON_MEMORY_REGION(addr, size) \
  ((void)(addr), (void)(size))
#define AR_ASAN_UNPOISON_MEMORY_REGION(addr, size) \
  ((void)(addr), (void)(size))
#endif

// Unsigned 8-bit integer.
typedef unsigned char      U8;
// Unsigned 16-bit integer.
typedef unsigned short     U16;
// Unsigned 32-bit integer.
typedef unsigned int       U32;
// Unsigned 64-bit integer.
typedef unsigned long long U64;

// Signed 8-bit integer.
typedef signed char      I8;
// Signed 16-bit integer.
typedef signed short     I16;
// Signed 32-bit integer.
typedef signed int       I32;
// Signed 64-bit integer.
typedef signed long long I64;

// 32-bit floating point number.
typedef float  F32;
// 64-bit floating point number.
typedef double F64;

// 8-bit boolean.
typedef U8 B8;
// 32-bit boolean.
typedef U32 B32;

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define ar_arrlen(ARR) (sizeof(ARR) / sizeof(ARR[0]))

#define ar_offsetof(T, M) ((U64) (void *) &((T *) NULL)->M)

#define ar_max(a, b) ((a) > (b) ? (a) : (b))
#define ar_min(a, b) ((a) < (b) ? (a) : (b))
#define ar_clamp(v, min, max) ((v) < (min) ? (min) : (v) > (max) ? (max) : (v))
#define ar_lerp(a, b, t) ((a) + ((b) - (a)) * (t))

static const U8  U8_MAX  = ~((U8)  0);
static const U16 U16_MAX = ~((U16) 0);
static const U32 U32_MAX = ~((U32) 0);
static const U64 U64_MAX = ~((U64) 0);

static const I8  I8_MIN  = (I8)  (1 << 7);
static const I16 I16_MIN = (I16) (1 << 15);
static const I32 I32_MIN = (I32)  1 << 31;
static const I64 I64_MIN = (I64) (1ll << 63);

static const I8  I8_MAX  = (I8)  ~0 ^ I8_MIN;
static const I16 I16_MAX = (I16) ~0 ^ I16_MIN;
static const I32 I32_MAX = (I32) ~0 ^ I32_MIN;
static const I64 I64_MAX = (I64) ~0 ^ I64_MIN;

#define KiB(value) (value << 10)
#define MiB(value) (value << 20)
#define GiB(value) ((U64) value << 30)

#define KB(value) (value * 1000)
#define MB(value) (value * 1000000)
#define GB(value) ((U64) value * 1000000000)

typedef struct ArStr ArStr;

typedef struct ArkinCoreDesc ArkinCoreDesc;
struct ArkinCoreDesc {
    U32 thread_pool_capacity;
    U32 mutex_pool_capacity;

    struct {
        void (*callback)(ArStr error);
    } error;

    struct {
        U64 default_capacity;
        U64 default_align;
    } arena;
};

// Initializes global state needed by other arkin function calls.
// This should be called at the start of a program by the main thread.
// It will create a thread context making scratch arenas available to the main
// thread.
ARKIN_API void arkin_init(const ArkinCoreDesc *desc);

// This should be called at the end of a program by the main thread as it will
// destroy the thread context and clean up other resources.
ARKIN_API void arkin_terminate(void);

//
// Arena
//

typedef struct ArArena ArArena;

ARKIN_API ArArena *ar_arena_create(U64 capacity);
// Uses a default capacity of 4 GiB.
ARKIN_API ArArena *ar_arena_create_default(void);
ARKIN_API void ar_arena_destroy(ArArena **arena);

ARKIN_API void ar_arena_set_align(ArArena *arena, U64 align);

// Returns a zero initialized region of memory.
ARKIN_API void *ar_arena_push(ArArena *arena, U64 size);
// Returns an uninitialized region of memory.
ARKIN_API void *ar_arena_push_no_zero(ArArena *arena, U64 size);
ARKIN_API void ar_arena_pop(ArArena *arena, U64 size);
ARKIN_API void ar_arena_reset(ArArena *arena);

ARKIN_API U64 ar_arena_used(const ArArena *arena);

#define ar_arena_push_arr(arena, type, len) ar_arena_push((arena), sizeof(type) * len)
#define ar_arena_push_arr_no_zero(arena, type, len) ar_arena_push_no_zero((arena), sizeof(type) * len)
#define ar_arena_push_type(arena, type) ar_arena_push((arena), sizeof(type))
#define ar_arena_push_type_no_zero(arena, type) ar_arena_push_no_zero((arena), sizeof(type))

typedef struct ArTemp ArTemp;
struct ArTemp {
    ArArena *const arena;
    const U32 pos;
};

ARKIN_API ArTemp ar_temp_begin(ArArena *arena);
ARKIN_API void ar_temp_end(ArTemp *temp);

ARKIN_API ArTemp ar_scratch_get(ArArena **conflicting, U32 count);
ARKIN_INLINE void ar_scratch_release(ArTemp *scratch) { ar_temp_end(scratch); }

//
// Thread context
//

typedef struct ArThreadCtx ArThreadCtx;

ARKIN_API ArThreadCtx *ar_thread_ctx_create(void);
ARKIN_API void ar_thread_ctx_destroy(ArThreadCtx **ctx);
ARKIN_API void ar_thread_ctx_set(ArThreadCtx *ctx);

//
// Linked lists
//

#define ar_null_set(p) ((p) = 0)
#define ar_null_check(p) ((p) == 0)

// Doubly linked list
#define ar_dll_insert(f, l, n, p) ar_dll_insert_npz(f, l, n, p, next, prev, ar_null_check, ar_null_set)
#define ar_dll_push_back(f, l, n) ar_dll_insert_npz(f, l, n, l, next, prev, ar_null_check, ar_null_set)
#define ar_dll_push_front(f, l, n) ar_dll_insert_npz(f, l, n, (__typeof__(n)) 0, next, prev, ar_null_check, ar_null_set)

#define ar_dll_remove(f, l, n) ar_dll_remove_npz(f, l, n, next, prev, ar_null_check, ar_null_set)
#define ar_dll_pop_back(f, l) ar_dll_remove_npz(f, l, l, next, prev, ar_null_check, ar_null_set)
#define ar_dll_pop_front(f, l) ar_dll_remove_npz(f, l, f, next, prev, ar_null_check, ar_null_set)

#define ar_dll_insert_npz(f, l, n, p, next, prev, zero_check, zero_set) do { \
    if (zero_check(f)) { \
        (f) = (l) = (n); \
        zero_set((n)->next); \
        zero_set((n)->prev); \
    } else { \
        if (zero_check(p)) { \
            (n)->next = (f); \
            zero_set((n)->prev); \
            (f)->prev = (n); \
            (f) = (n); \
        } else { \
            if (!zero_check((p)->next)) { \
                (p)->next->prev = (n); \
                (n)->next = (p)->next; \
            } \
            (n)->prev = (p); \
            (p)->next = (n); \
            if ((p) == (l)) { \
                (l) = (n); \
            } \
        } \
    } \
} while (0)
#define ar_dll_push_back_npz(f, l, n, next, prev, zero_check, zero_set) ar_dll_insert_npz(f, l, n, l, next, prev, zero_check, zero_set)
#define ar_dll_push_front_npz(f, l, n, next, prev, zero_check, zero_set) ar_dll_insert_npz(f, l, n, (__typeof__(n)) 0, next, prev, zero_check, zero_set)

#define ar_dll_remove_npz(f, l, n, next, prev, zero_check, zero_set) do { \
    if (!zero_check(f)) { \
        if ((f) == (l)) { \
            zero_set(f); \
            zero_set(l); \
        } else { \
            if (!zero_check((n)->next)) { \
                (n)->next->prev = (n)->prev; \
            } \
            if (!zero_check((n)->prev)) { \
                (n)->prev->next = (n)->next; \
            } \
            if ((n) == (f)) { \
                if (!zero_check((f)->next)) { \
                    (f)->next->prev = (f)->prev; \
                } \
                (f) = (f)->next; \
            } \
            if ((n) == (l)) { \
                if (!zero_check((l)->prev)) { \
                    (l)->prev->next = (l)->next; \
                } \
                (l) = (l)->prev; \
            } \
        } \
    } \
} while (0)
#define ar_dll_pop_back_npz(f, l, n, next, prev, zero_check, zero_set) ar_dll_remove_npz(f, l, l, next, prev, zero_check, zero_set)
#define ar_dll_pop_front_npz(f, l, n, next, prev, zero_check, zero_set) ar_dll_remove_npz(f, l, f, next, prev, zero_check, zero_set)

// Singly linked list
#define ar_sll_queue_push(f, l, n) ar_sll_queue_push_nz(f, l, n, next, ar_null_check, ar_null_set)
#define ar_sll_queue_pop(f, l) ar_sll_queue_pop_nz(f, l, next, ar_null_check, ar_null_set)

#define ar_sll_queue_push_nz(f, l, n, next, zero_check, zero_set) do { \
    if (zero_check(f)) { \
        (f) = (l) = (n); \
    } else { \
        (l)->next = (n); \
        (l) = (n); \
    } \
    (n)->next = NULL; \
} while (0)
#define ar_sll_queue_pop_nz(f, l, next, zero_check, zero_set) do { \
    if ((f) == (l)) { \
        zero_set(f); \
        zero_set(l); \
    } else { \
        (f) = (f)->next; \
    }\
} while (0)

#define ar_sll_stack_push(f, n) ar_sll_stack_push_nz(f, n, next, ar_null_check)
#define ar_sll_stack_pop(f) ar_sll_stack_pop_nz(f, next, ar_null_check)

#define ar_sll_stack_push_nz(f, n, next, zero_check) do { \
    if (!zero_check(f)) { \
        n->next = f; \
    } \
    f = n; \
} while (0)
#define ar_sll_stack_pop_nz(f, next, zero_check) do { \
    if (!zero_check(f)) { \
        f = f->next; \
    } \
} while (0)

//
// Strings
//

// Char helpers
ARKIN_API B8 ar_char_is_numeric(char c);
ARKIN_API B8 ar_char_is_alpha(char c);
ARKIN_API B8 ar_char_is_lower(char c);
ARKIN_API B8 ar_char_is_upper(char c);
ARKIN_API B8 ar_char_is_whitespace(char c);
ARKIN_API char ar_char_to_lower(char c);
ARKIN_API char ar_char_to_upper(char c);

// Non-destructive length based strings.
// No operation shall change the data of the string, instead producing a new
// one.

struct ArStr {
    U64 len;
    const U8 *data;
};

typedef enum {
    AR_STR_MATCH_FLAG_EXACT,

    AR_STR_MATCH_FLAG_LAST = 1 << 0,
    AR_STR_MATCH_FLAG_SLOPPY_LENGTH = 1 << 1,
    AR_STR_MATCH_FLAG_CASE_INSENSITIVE = 1 << 2,

    AR_STR_MATCH_FLAG_COUNT,
} ArStrMatchFlag;

// String list

typedef struct ArStrListNode ArStrListNode;
struct ArStrListNode {
    ArStrListNode *next;
    ArStrListNode *prev;
    ArStr str;
};

typedef struct ArStrList ArStrList;
struct ArStrList {
    ArStrListNode *first;
    ArStrListNode *last;
};

static const ArStrList AR_STR_LIST_INIT = {0};

// Pushes string into the back of the string list.
ARKIN_API void ar_str_list_push(ArArena *arena, ArStrList *list, ArStr str);
// Pushes string into the front of the string list.
ARKIN_API void ar_str_list_push_front(ArArena *arena, ArStrList *list, ArStr str);

// Pops string from the back of the string list.
ARKIN_API void ar_str_list_pop(ArStrList *list);
// Pops string from the back of the string list.
ARKIN_API void ar_str_list_pop_front(ArStrList *list);

ARKIN_API ArStr ar_str_list_join(ArArena *arena, ArStrList list);

#define ar_str_lit(str) (ArStr) { sizeof(str) - 1, (const U8 *) str }
#define ar_str_cstr(str) (ArStr) { strlen(str), (const U8 *) str }
#define ar_str(str, len) (ArStr) { len, str }

ARKIN_API ArStr ar_str_pushf(ArArena *arena, const char *fmt, ...) AR_FORMAT_FUNCTION(2, 3);
ARKIN_API ArStr ar_str_pushfv(ArArena *arena, const char *fmt, va_list args);
ARKIN_API ArStr ar_str_push_copy(ArArena *arena, ArStr str);

// If sloppy length is specified, smallest length will be used to match.
ARKIN_API B8 ar_str_match(ArStr a, ArStr b, ArStrMatchFlag flags);

// Inclusive range of start and end.
ARKIN_API ArStr ar_str_sub(ArStr str, U64 start, U64 end);
// Inclusive range of start and end.
ARKIN_API ArStr ar_str_sub_len(ArStr str, U64 start, U64 len);

ARKIN_INLINE ArStr ar_str_chop_start(ArStr str, U64 len) {
    return ar_str_sub(str, len, str.len - 1);
}
ARKIN_INLINE ArStr ar_str_chop_end(ArStr str, U64 len) {
    return ar_str_sub(str, 0, str.len - len - 1);
}

// Returns haystack length if no needle was found.
ARKIN_API U64 ar_str_find(ArStr haystack, ArStr needle, ArStrMatchFlag flags);
ARKIN_API U64 ar_str_find_char(ArStr haystack, char needle, ArStrMatchFlag flags);

// Trim both beginning and end of a string.
ARKIN_API ArStr ar_str_trim(ArStr str);
// Trim beginning end of a string.
ARKIN_API ArStr ar_str_trim_front(ArStr str);
// Trim end end of a string.
ARKIN_API ArStr ar_str_trim_back(ArStr str);

ARKIN_API ArStrList ar_str_split(ArArena *arena, ArStr str, ArStr delim, ArStrMatchFlag flags);
ARKIN_API ArStrList ar_str_split_char(ArArena *arena, ArStr str, char delim, ArStrMatchFlag flags);

//
// Hash map
//

ARKIN_API B8 ar_memeq(const void *a, const void *b, U64 len);
ARKIN_API U64 ar_fvn1a_hash(const void *data, U64 len);

typedef U64 (*ArHashFunc)(const void *data, U64 len);
typedef B8 (*ArHashMapEqualFunc)(const void *a, const void *b, U64 len);

typedef struct ArHashMapDesc ArHashMapDesc;
struct ArHashMapDesc {
    ArArena *arena;
    U32 capacity;

    ArHashFunc hash_func;
    ArHashMapEqualFunc eq_func;

    U64 key_size;
    U64 value_size;
    const void *null_value;
};

typedef struct ArHashMap ArHashMap;

// Initializes a new hash map.
// If the key-value is something living on the
// heap it is recommended to clone it onto the hash map arena before using
// it in a set or insert operation. If the memory becomes invalid while the
// hash map is still in use, something might break.
ARKIN_API ArHashMap *ar_hash_map_init(ArHashMapDesc desc);

// Getter for hash map arena.
ARKIN_API ArArena *ar_hash_map_get_arena(const ArHashMap *map);

// Inserts a unique key-value pair into the hash map.
// Returns true if operation was successful, false if key was already present
// within the hash map.
#define ar_hash_map_insert(map, key, value) ({ \
    __typeof__(key) _ar_hm_temp_key = key; \
    __typeof__(value) _ar_hm_temp_value = value; \
    _ar_hash_map_insert(map, &_ar_hm_temp_key, &_ar_hm_temp_value); \
})

// Sets the value of key. If key isn't present in the hash map it is inserted,
// if it does, its value is updated.
#define ar_hash_map_set(map, key, value) ({ \
    __typeof__(key) _ar_hm_temp_key = key; \
    __typeof__(value) _ar_hm_temp_value = value; \
    _ar_hash_map_set(map, &_ar_hm_temp_key, &_ar_hm_temp_value); \
})

// Removes a key-value pair from the hash map.
// Return true if a pair was removed. Returns false if no pair with the same
// key was found.
#define ar_hash_map_remove(map, key) ({ \
    __typeof__(key) _ar_hm_temp_key = key; \
    _ar_hash_map_remove(map, &_ar_hm_temp_key); \
})

// Checks whether a key is present in the hash map.
#define ar_hash_map_has(map, key) ({ \
    __typeof__(key) _ar_hm_temp_key = key; \
    _ar_hash_map_remove(map, &_ar_hm_temp_key); \
})

// Gets a value from a key-value pair within the hash map. Returns the null
// value specified when initializing the hash map if no pair was found.
#define ar_hash_map_get(map, key, value_type) ({ \
    __typeof__(key) _ar_hm_temp_key = key; \
    value_type _ar_hm_temp_return_value; \
    _ar_hash_map_get(map, &_ar_hm_temp_key, &_ar_hm_temp_return_value); \
    _ar_hm_temp_return_value; \
})

// Gets a pointer to the interval storage place for the key. Returns null
// if the key isn't present within the hash map.
#define ar_hash_map_get_ptr(map, key) ({ \
    __typeof__(key) _ar_hm_temp_key = key; \
    _ar_hash_map_get_ptr(map, &_ar_hm_temp_key); \
})

// Private API.
// Recommended to not touch this unless you know what's going on underneath
// the hood.
ARKIN_API B8 _ar_hash_map_insert(ArHashMap *map, const void *key, const void *value);
ARKIN_API B8 _ar_hash_map_set(ArHashMap *map, const void *key, const void *value);
ARKIN_API B8 _ar_hash_map_remove(ArHashMap *map, const void *key);
ARKIN_API B8 _ar_hash_map_has(const ArHashMap *map, const void *key);
ARKIN_API void _ar_hash_map_get(const ArHashMap *map, const void *key, void *output);
ARKIN_API void * _ar_hash_map_get_ptr(const ArHashMap *map, const void *key);

//
// Pool allocator
//

typedef struct ArPool ArPool;
typedef struct ArPoolHandle ArPoolHandle;
struct ArPoolHandle {
    U64 handle;
};

static const ArPoolHandle AR_POOL_HANDLE_INVALID = {U64_MAX};

ARKIN_API ArPool *ar_pool_init(ArArena *arena, U32 capacity, U64 object_size);
ARKIN_API ArPoolHandle ar_pool_handle_create(ArPool *pool);
ARKIN_API void ar_pool_handle_destroy(ArPool *pool, ArPoolHandle handle);
ARKIN_API B8 ar_pool_handle_valid(const ArPool *pool, ArPoolHandle handle);
ARKIN_API void *ar_pool_handle_to_ptr(const ArPool *pool, ArPoolHandle handle);

ARKIN_API ArPoolHandle ar_pool_iter_init(const ArPool *pool);
ARKIN_API B8 ar_pool_iter_valid(const ArPool *pool, ArPoolHandle iter);
ARKIN_API ArPoolHandle ar_pool_iter_next(const ArPool *pool, ArPoolHandle iter);

//
// Error handling
//

//
// Thread local error handling.
// If a thread doesn't have a thread contxt this won't work.
//

typedef enum {
    AR_ERR_ACCUM_TYPE_IGNORE,
    AR_ERR_ACCUM_TYPE_FIRST,
    AR_ERR_ACCUM_TYPE_STACK,
} ArErrAccumType;

typedef struct ArErr ArErr;
struct ArErr {
    ArErr *next;
    ArStr str;
};

ARKIN_API void ar_err_accum_begin(ArErrAccumType type);
ARKIN_API ArErr *ar_err_accum_end(ArArena *arena);

ARKIN_API void ar_err_emit(ArStr str);
ARKIN_API void ar_err_emitf(const char *fmt, ...);

//
// Platform
//

ARKIN_API F64 ar_os_get_time(void);
ARKIN_API U64 ar_os_get_time_microseconds(void);
ARKIN_API U32 ar_os_page_size(void);

// Reserves 'size', aligned upwards to the next page boundy, of memory
// addresses.
//
// It adds a size of 3 U64 to the size for the allocation header
// used internally.
ARKIN_API void *ar_os_mem_reserve(U64 size);

// Grows readble and writable size of 'ptr' upwards.
//
// Doing pointer arithmatic on 'ptr' is strongly discouraged due to internal
// arithmatic to get allocation info.
ARKIN_API void ar_os_mem_commit(void *ptr, U64 size);

// Shrinks readable and writable chunk of 'ptr' downwards.
// There will always be a (pagesize - sizeof(U64)*3) chunk of readable and
// writable memory available until it is released.
//
// Doing pointer arithmatic on 'ptr' is strongly discouraged due to internal
// arithmatic to get allocation info.
ARKIN_API void ar_os_mem_decommit(void *ptr, U64 size);

// Releases the all the reserved address space back to the OS.
ARKIN_API void ar_os_mem_release(void *ptr);

//
// Threads
//

typedef struct ArThread ArThread;
struct ArThread {
    ArPoolHandle handle;
};

typedef void (*ArThreadFunc)(void *args);

// Starts a thread.
ARKIN_API ArThread ar_thread_create(ArThreadFunc func, void *args);
// Starts a thread without creating a thread context.
// Without a thread context scratch arenas won't be available.
ARKIN_API ArThread ar_thread_create_no_ctx(ArThreadFunc func, void *args);
// Cleans up the OS specific thread things and frees up space in the thread pool.
ARKIN_API void ar_thread_destroy(ArThread thread);
// Waits for the thread to finish. Destroys the thread. Don't call ar_thread_destroy after calling this function.
ARKIN_API void ar_thread_join(ArThread thread);
// Detaches thread from program. Destroys the thread. Don't call ar_thread_destroy after calling this function.
ARKIN_API void ar_thread_detach(ArThread thread);
ARKIN_API B8 ar_thread_valid(ArThread thread);

//
// Mutexes
//

typedef struct ArMutex ArMutex;
struct ArMutex {
    ArPoolHandle handle;
};

ARKIN_API ArMutex ar_mutex_create(void);
ARKIN_API void ar_mutex_destroy(ArMutex mutex);
ARKIN_API void ar_mutex_lock(ArMutex mutex);
ARKIN_API void ar_mutex_unlock(ArMutex mutex);
ARKIN_API B8 ar_mutex_valid(ArMutex mutex);

#endif
