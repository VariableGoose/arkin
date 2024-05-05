#ifndef ARKIN_LOG_H
#define ARKIN_LOG_H

#include "arkin_core.h"
#include <stdio.h>

#ifndef ARKIN_LOG_MAX_MESSAGE_LENGTH
#define ARKIN_LOG_MAX_MESSAGE_LENGTH 2048
#endif

#ifndef ARKIN_LOG_MAX_CALLBACK_COUNT
#define ARKIN_LOG_MAX_CALLBACK_COUNT 16
#endif

#define al_fatal(msg, ...) _al_log(AL_LOG_LEVEL_FATAL, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_fatal(msg, ...) _al_log(AL_LOG_LEVEL_FATAL, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_error(msg, ...) _al_log(AL_LOG_LEVEL_ERROR, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_warn(msg, ...)  _al_log(AL_LOG_LEVEL_WARN,  __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_info(msg, ...)  _al_log(AL_LOG_LEVEL_INFO,  __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_debug(msg, ...) _al_log(AL_LOG_LEVEL_DEBUG, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_trace(msg, ...) _al_log(AL_LOG_LEVEL_TRACE, __FILE__, __LINE__, msg, ##__VA_ARGS__)

typedef enum {
    AL_LOG_LEVEL_FATAL,
    AL_LOG_LEVEL_ERROR,
    AL_LOG_LEVEL_WARN,
    AL_LOG_LEVEL_INFO,
    AL_LOG_LEVEL_DEBUG,
    AL_LOG_LEVEL_TRACE,

    AL_LOG_LEVEL_COUNT,
} al_log_level_t;

typedef void (*al_callback_t)(al_log_level_t level, const char *file, u32_t line, const char *msg, void *userdata);
ARKIN_API void al_add_callback(al_callback_t callback, al_log_level_t level, b8_t color, void *userdata);
ARKIN_API void al_add_fp(al_log_level_t level, b8_t color, FILE *fp);

ARKIN_API void _al_log(al_log_level_t level, const char *file, u32_t line, const char *fmt, ...);

typedef struct _arkin_log_state_t _arkin_log_state_t;
struct _arkin_log_state_t {
    struct {
        al_callback_t func;
        al_log_level_t level;
        b8_t color;
        void *userdata;
    } callbacks[ARKIN_LOG_MAX_CALLBACK_COUNT];
    u32_t callback_count;
};
extern _arkin_log_state_t _al_state;

#endif
