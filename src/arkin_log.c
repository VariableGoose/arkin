#include "arkin_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

_arkin_log_state_t _al_state = {0};

static char _msg[ARKIN_LOG_MAX_MESSAGE_LENGTH] = {0};
static char _msg_color[ARKIN_LOG_MAX_MESSAGE_LENGTH] = {0};

void _al_log(al_log_level_t level, const char *file, u32_t line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    static char user_message[ARKIN_LOG_MAX_MESSAGE_LENGTH] = {0};
    vsnprintf(user_message, ARKIN_LOG_MAX_MESSAGE_LENGTH, fmt, args);

    va_end(args);

    const char *level_string[AL_LOG_LEVEL_COUNT] = {
        "FATAL",
        "ERROR",
        "WARN ",
        "INFO ",
        "DEBUG",
        "TRACE",
    };

    const char *level_string_color[AL_LOG_LEVEL_COUNT] = {
        "\e[1;101m",
        "\e[1;91m",
        "\e[0;93m",
        "\e[0;92m",
        "\e[0;94m",
        "\e[0;95m",
    };

    i32_t ret = snprintf(_msg, ARKIN_LOG_MAX_MESSAGE_LENGTH, "%s %s:%d: %s\n", level_string[level], file, line, user_message);
    // Idk why but this gets rid of some truncation warning.
    (void) ret;

    ret = snprintf(_msg_color, ARKIN_LOG_MAX_MESSAGE_LENGTH, "%s%s\e[0;37m %s:%d:\e[0;0m %s\n", level_string_color[level], level_string[level], file, line, user_message);

    for (u32_t i = 0; i < _al_state.callback_count; i++) {
        if (level <= _al_state.callbacks[i].level) {
            const char *msg = _al_state.callbacks[i].level ? _msg_color : _msg;
            _al_state.callbacks[i].func(level, file, line, msg, _al_state.callbacks[i].userdata);
        }
    }
}

void al_add_callback(al_callback_t callback, al_log_level_t level, b8_t color, void *userdata) {
    _al_state.callbacks[_al_state.callback_count].func = callback;
    _al_state.callbacks[_al_state.callback_count].level = level;
    _al_state.callbacks[_al_state.callback_count].color = color;
    _al_state.callbacks[_al_state.callback_count].userdata = userdata;
    _al_state.callback_count++;
}

static void _al_file_callback(al_log_level_t level, const char *file, u32_t line, const char *msg, void *userdata) {
    FILE *fp = userdata;
    fwrite(msg, strlen(msg), 1, fp);
}

void al_add_fp(al_log_level_t level, b8_t color, FILE *fp) {
    al_add_callback(_al_file_callback, level, color, fp);
}
