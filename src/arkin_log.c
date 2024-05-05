#include "arkin_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

_arkin_log_state_t _al_state = {0};

static void al_log_stdout(al_log_event_t event) {
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

    if (!_al_state.no_stdout_color) {
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

void _al_log(al_log_level_t level, const char *file, u32_t line, const char *fmt, ...) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    al_log_event_t event = {
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
    vsnprintf(event.message, arrlen(event.message), fmt, args);
    va_end(args);

    if (!_al_state.no_stdout) {
        al_log_stdout(event);
    }

    for (u32_t i = 0; i < _al_state.callback_count; i++) {
        if (level <= _al_state.callbacks[i].level) {
            _al_state.callbacks[i].func(event, _al_state.callbacks[i].userdata);
        }
    }
}

void al_add_callback(al_callback_t callback, al_log_level_t level, void *userdata) {
    _al_state.callbacks[_al_state.callback_count].func = callback;
    _al_state.callbacks[_al_state.callback_count].level = level;
    _al_state.callbacks[_al_state.callback_count].userdata = userdata;
    _al_state.callback_count++;
}

static void _al_file_callback(al_log_event_t event, void *userdata) {
    FILE *fp = userdata;
    fprintf(fp, "%s\n", event.message);
}

void al_add_fp(al_log_level_t level, FILE *fp) {
    al_add_callback(_al_file_callback, level, fp);
}

void al_set_no_stdout(b8_t value) {
    _al_state.no_stdout = value;
}

void al_set_no_stdout_color(b8_t value) {
    _al_state.no_stdout_color = value;
}
