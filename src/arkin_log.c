#include "arkin_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

_ArkinLogState _ar_state = {0};

static void ar_log_stdout(ArLogEvent event) {
    static const char *level_string[AL_LOG_LEVEL_COUNT] = {
        "FATAL",
        "ERROR",
        "WARN ",
        "INFO ",
        "DEBUG",
        "TRACE",
    };

    static const char *level_string_color[AL_LOG_LEVEL_COUNT] = {
        "\e[0;0m\e[1;101m",
        "\e[1;91m",
        "\e[0;93m",
        "\e[0;92m",
        "\e[0;94m",
        "\e[0;95m",
    };

    if (!_ar_state.no_stdout_color) {
        printf("\e[0;37m%.2u:%.2u:%.2u %s%s\e[0;37m %s:%d:\e[0;0m %s\n",
                event.hour,
                event.minute,
                event.second,
                level_string_color[event.level],
                level_string[event.level],
                event.file,
                event.line,
                event.message);
    } else {
        printf("%.2u:%.2u:%.2u %s %s:%d: %s\n",
                event.hour,
                event.minute,
                event.second,
                level_string[event.level],
                event.file,
                event.line,
                event.message);
    }
}

void _ar_log(ArLogLevel level, const char *file, U32 line, const char *fmt, ...) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    ArLogEvent event = {
        .message = {0},
        .level = level,
        .hour = tm->tm_hour,
        .minute = tm->tm_min,
        .second = tm->tm_sec,
        .file = file,
        .line = line,
    };

    va_list args;
    va_start(args, fmt);
    vsnprintf(event.message, ar_arrlen(event.message), fmt, args);
    va_end(args);

    if (!_ar_state.no_stdout) {
        ar_log_stdout(event);
    }

    for (U32 i = 0; i < _ar_state.callback_count; i++) {
        if (level <= _ar_state.callbacks[i].level) {
            _ar_state.callbacks[i].func(event, _ar_state.callbacks[i].userdata);
        }
    }
}

void ar_add_callback(ArLogCallback callback, ArLogLevel level, void *userdata) {
    _ar_state.callbacks[_ar_state.callback_count].func = callback;
    _ar_state.callbacks[_ar_state.callback_count].level = level;
    _ar_state.callbacks[_ar_state.callback_count].userdata = userdata;
    _ar_state.callback_count++;
}

static void _ar_file_callback(ArLogEvent event, void *userdata) {
    FILE *fp = userdata;
    fprintf(fp, "%s\n", event.message);
}

void ar_add_fp(ArLogLevel level, FILE *fp) {
    ar_add_callback(_ar_file_callback, level, fp);
}

void ar_set_no_stdout(B8 value) {
    _ar_state.no_stdout = value;
}

void ar_set_no_stdout_color(B8 value) {
    _ar_state.no_stdout_color = value;
}
