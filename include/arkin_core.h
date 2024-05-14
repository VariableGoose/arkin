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

#ifndef arrlen
#define arrlen(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#endif

#ifndef offsetof
#define offsetof(T, M) ((U64) (void *) &((T *) NULL)->M)
#endif

#define AC_MALLOC(SIZE) _ac.malloc(SIZE, __FILE__, __LINE__)
#define AC_REALLOC(PTR, SIZE) _ac.realloc(PTR, SIZE, __FILE__, __LINE__)
#define AC_FREE(PTR) _ac.free(PTR, __FILE__, __LINE__)

typedef struct ArkinCoreDesc ArkinCoreDesc;
struct ArkinCoreDesc {
    void *(*malloc)(U64 size, const char *file, U32 line);
    void *(*realloc)(void *ptr, U64 size, const char *file, U32 line);
    void (*free)(void *ptr, const char *file, U32 line);
};

ARKIN_API void arkin_init(const ArkinCoreDesc *desc);
ARKIN_API void arkin_terminate(void);

typedef struct _ArkinCoreState _ArkinCoreState;
struct _ArkinCoreState {
    void *(*malloc)(U64 size, const char *file, U32 line);
    void *(*realloc)(void *ptr, U64 size, const char *file, U32 line);
    void (*free)(void *ptr, const char *file, U32 line);
};
extern _ArkinCoreState _ac;

#endif
