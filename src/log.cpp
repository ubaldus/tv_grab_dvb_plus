#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"

extern char *ProgName;

static int level = ERROR;

static const char *log_label(int l) {
    switch (l) {
    case INFO:
        return "INFO    ";
    case TRACE:
        return "TRACE   ";
    case DEBUG :
        return "DEBUG   ";
    case WARNING:
        return "WARNING ";
    case ERROR:
        return "ERROR   ";
    default:
    	/* should not get here! */
        return "";
    }
}

void log_level(char *l) {
    if (!strcasecmp(l, "INFO")) {
        level = INFO;
    } else if (!strcasecmp(l, "TRACE")) {
        level = TRACE;
    } else if (!strcasecmp(l, "DEBUG")) {
        level = DEBUG;
    } else if (!strcasecmp(l, "WARNING")) {
        level = WARNING;
    } else if (!strcasecmp(l, "ERROR")) {
        level = ERROR;
    } else if (!strcasecmp(l, "NONE")) {
        level = NONE;
    } else {
        level = ERROR;
    }
}

int is_logging(int l) {
    if (l >= level) {
        return 1;
    } else {
        return 0;
    }
}

void log_message(int l, const char *format, ...) {
    va_list args;

    va_start(args, format);
    if (l >= level) {
        fprintf(stderr, "%s %s", ProgName, log_label(l));
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
    }
    va_end(args);
}
