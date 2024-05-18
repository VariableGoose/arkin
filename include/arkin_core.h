#ifndef ARKIN_CORE_H
#define ARKIN_CORE_H

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

#ifndef ar_arrlen
#define ar_arrlen(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#endif

#ifndef ar_offsetof
#define ar_offsetof(T, M) ((U64) (void *) &((T *) NULL)->M)
#endif

#define AR_MALLOC(SIZE) _ar_core.malloc(SIZE, __FILE__, __LINE__)
#define AR_REALLOC(PTR, SIZE) _ar_core.realloc(PTR, SIZE, __FILE__, __LINE__)
#define AR_FREE(PTR) _ar_core.free(PTR, __FILE__, __LINE__)

typedef struct ArkinCoreDesc ArkinCoreDesc;
struct ArkinCoreDesc {
    void *(*malloc)(U64 size, const char *file, U32 line);
    void *(*realloc)(void *ptr, U64 size, const char *file, U32 line);
    void (*free)(void *ptr, const char *file, U32 line);
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
// Uses a default capacity of 4 GiB. (4 * 1 << 30).
ARKIN_API ArArena *ar_arena_create_default(void);
ARKIN_API void ar_arena_destroy(ArArena **arena);

ARKIN_API void *ar_arena_push(ArArena *arena, U64 size);
ARKIN_API void ar_arena_pop(ArArena *arena, U64 size);
ARKIN_API void ar_arena_reset(ArArena *arena);

ARKIN_API U64 ar_arena_used(const ArArena *arena);

#define ar_arena_push_arr(arena, type, len) ar_arena_push((arena), sizeof(type) * len)
#define ar_arena_push_type(arena, type) ar_arena_push((arena), sizeof(type))

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
// Platform
//

ARKIN_API F64 ar_os_get_time(void);
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
    U64 handle;
};

typedef void (*ArThreadFunc)(void *args);

ARKIN_API ArThread ar_thread_start(ArThreadFunc func, void *args);
// Starts a thread without creating a thread context.
// Without a thread context scratch arenas won't be available.
ARKIN_API ArThread ar_thread_start_no_ctx(ArThreadFunc func, void *args);
ARKIN_API ArThread ar_thread_current(void);
ARKIN_API void ar_thread_join(ArThread thread);
ARKIN_API void ar_thread_detatch(ArThread thread);

ARKIN_API void _ar_os_init(void);
ARKIN_API void _ar_os_terminate(void);

typedef struct _ArkinCoreState _ArkinCoreState;
struct _ArkinCoreState {
    void *(*malloc)(U64 size, const char *file, U32 line);
    void *(*realloc)(void *ptr, U64 size, const char *file, U32 line);
    void (*free)(void *ptr, const char *file, U32 line);

    ArThreadCtx *thread_ctx;
};
extern _ArkinCoreState _ar_core;

#endif
