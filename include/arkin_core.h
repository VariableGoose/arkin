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

// Unsigned 8-bit integer.
typedef unsigned char      u8_t;
// Unsigned 16-bit integer.
typedef unsigned short     u16_t;
// Unsigned 32-bit integer.
typedef unsigned int       u32_t;
// Unsigned 64-bit integer.
typedef unsigned long long u64_t;

// Signed 8-bit integer.
typedef signed char      i8_t;
// Signed 16-bit integer.
typedef signed short     i16_t;
// Signed 32-bit integer.
typedef signed int       i32_t;
// Signed 64-bit integer.
typedef signed long long i64_t;

// 32-bit floating point number.
typedef float  f32_t;
// 64-bit floating point number.
typedef double f64_t;

// 8-bit boolean.
typedef u8_t b8_t;
// 32-bit boolean.
typedef u32_t b32_t;

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef arrlen
#define arrlen(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#endif

#ifndef offsetof
#define offsetof(T, M) ((u64_t) (void *) &((T *) NULL)->M)
#endif

#define AC_MALLOC(SIZE) _ac.malloc(SIZE, __FILE__, __LINE__)
#define AC_REALLOC(PTR, SIZE) _ac.realloc(PTR, SIZE, __FILE__, __LINE__)
#define AC_FREE(PTR) _ac.free(PTR, __FILE__, __LINE__)

typedef struct arkin_core_desc_t arkin_core_desc_t;
struct arkin_core_desc_t {
    void *(*malloc)(u64_t size, const char *file, u32_t line);
    void *(*realloc)(void *ptr, u64_t size, const char *file, u32_t line);
    void (*free)(void *ptr, const char *file, u32_t line);
};

ARKIN_API void arkin_init(const arkin_core_desc_t *desc);
ARKIN_API void arkin_terminate(void);

typedef struct _arkin_core_state_t _arkin_core_state_t;
struct _arkin_core_state_t {
    void *(*malloc)(u64_t size, const char *file, u32_t line);
    void *(*realloc)(void *ptr, u64_t size, const char *file, u32_t line);
    void (*free)(void *ptr, const char *file, u32_t line);
};
extern _arkin_core_state_t _ac;

#endif
