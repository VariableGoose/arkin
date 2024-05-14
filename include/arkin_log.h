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

typedef enum {
    AL_LOG_LEVEL_FATAL,
    AL_LOG_LEVEL_ERROR,
    AL_LOG_LEVEL_WARN,
    AL_LOG_LEVEL_INFO,
    AL_LOG_LEVEL_DEBUG,
    AL_LOG_LEVEL_TRACE,

    AL_LOG_LEVEL_COUNT,
} AlLogLevel;

typedef struct AlLogEvent AlLogEvent;
struct AlLogEvent {
    char message[ARKIN_LOG_MAX_MESSAGE_LENGTH];
    AlLogLevel level;
    U8 hour;
    U8 minute;
    U8 second;
    const char *file;
    U32 line;
};

typedef void (*AlCallback)(AlLogEvent event, void *userdata);

#define al_fatal(msg, ...) _al_log(AL_LOG_LEVEL_FATAL, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_fatal(msg, ...) _al_log(AL_LOG_LEVEL_FATAL, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_error(msg, ...) _al_log(AL_LOG_LEVEL_ERROR, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_warn(msg, ...)  _al_log(AL_LOG_LEVEL_WARN,  __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_info(msg, ...)  _al_log(AL_LOG_LEVEL_INFO,  __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_debug(msg, ...) _al_log(AL_LOG_LEVEL_DEBUG, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define al_trace(msg, ...) _al_log(AL_LOG_LEVEL_TRACE, __FILE__, __LINE__, msg, ##__VA_ARGS__)

ARKIN_API void al_add_callback(AlCallback callback, AlLogLevel level, void *userdata);
ARKIN_API void al_add_fp(AlLogLevel level, FILE *fp);
ARKIN_API void al_set_no_stdout(B8 value);
ARKIN_API void al_set_no_stdout_color(B8 value);

ARKIN_API void _al_log(AlLogLevel level, const char *file, U32 line, const char *fmt, ...);

typedef struct _ArkinLogState _ArkinLogState;
struct _ArkinLogState {
    struct {
        AlCallback func;
        AlLogLevel level;
        void *userdata;
    } callbacks[ARKIN_LOG_MAX_CALLBACK_COUNT];
    U32 callback_count;
    B8 no_stdout;
    B8 no_stdout_color;
};
extern _ArkinLogState _al_state;

#endif
