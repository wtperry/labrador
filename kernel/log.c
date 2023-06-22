#include <kernel/log.h>

#include <kernel/printf.h>

#define MAX_LOG_OUTPUTS 4
#define MAX_MSG_SIZE 256

typedef struct log_output {
    console_write_t write_func;
    int max_level;
} log_output_t;

log_output_t log_outputs[MAX_LOG_OUTPUTS];

static const char *get_level_string(int level) {
    switch (level)
    {
    case LOG_FATAL:
        return "[\033[35mFATAL\033[0m] ";
        break;
    
    case LOG_ERROR:
        return "[\033[31mERROR\033[0m] ";
        break;

    case LOG_WARNING:
        return "[\033[33mWARNING\033[0m] ";
        break;
    
    case LOG_INFO:
        return "[\033[32mINFO\033[0m] ";
        break;
    
    case LOG_DEBUG:
        return "[\033[36mDEBUG\033[0m] ";
        break;
    
    default:
        return "";
        break;
    }
}

void log_init(void) {
    for (size_t i = 0; i < MAX_LOG_OUTPUTS; i++) {
        log_outputs[i].write_func = NULL;
    }
}

int log_printf(int level, const char* restrict format, ...) {
    va_list parameters;
	va_start(parameters, format);

    return log_vprintf(level, format, parameters);
}

int log_vprintf(int level, const char* restrict format, va_list args) {
    char msg_buffer[MAX_MSG_SIZE];
    int msg_size;

    const char* level_string = get_level_string(level);

    msg_size = snprintf(msg_buffer, MAX_MSG_SIZE - 2, "%s", level_string);

    msg_size += vsnprintf(msg_buffer + msg_size, MAX_MSG_SIZE - 2 - msg_size, format, args);

    // Add a new line to the end of the log message
    msg_buffer[msg_size] = '\r';
    msg_buffer[msg_size + 1] = '\n';
    msg_buffer[msg_size + 2] = '\0';
    msg_size += 2;

    for (size_t i = 0; i < MAX_LOG_OUTPUTS; i++) {
        if (log_outputs[i].write_func && (level <= log_outputs[i].max_level)) {
            log_outputs[i].write_func(msg_buffer);
        }
    }

    return msg_size;
}

int log_add_output(int max_level, console_write_t write_func) {
    for (size_t i = 0; i < MAX_LOG_OUTPUTS; i++) {
        if (!log_outputs[i].write_func) {
            log_outputs[i].write_func = write_func;
            log_outputs[i].max_level = max_level;
            return 0;
        }
    }
    return 1;
}