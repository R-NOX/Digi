#include "log.h"
//#include "config.h"

#define _GNU_SOURCE
#include <stdio.h>
int asprintf(char **strp, const char *fmt, ...);
int vasprintf(char **strp, const char *fmt, va_list ap);

#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <jansson.h>
#include <stdarg.h>

/* Avoid "unused parameter" warnings */
#define _UNUSED(x) (void)x;

#define LOG_LOWER_PRIORITY_DAEMON_VAR       "log_lower_daemon_priority"
#define LOG_LOWER_PRIORITY_DAEMON_DEFAULT   LOG_INFO
#define LOG_LOWER_PRIORITY_PROGRAM_VAR      "log_lower_program_priority"
#define LOG_LOWER_PRIORITY_PROGRAM_DEFAULT  LOG_DEBUG
static const char *log_priority_names[8] = { 0 };


static bool isProgram = false;
static bool flagSyslog = true;
static bool flagStdout = true;
static bool flagStderr = true;


void log_slot(const char* key, const json_t *value, const char *filename);

void log_open(int __option, int __facility, bool __isProgram)
{
    // initialize an array of priority names
    log_priority_names[LOG_EMERG] = "emergency";
    log_priority_names[LOG_ALERT] = "alert";
    log_priority_names[LOG_CRIT] = "critical";
    log_priority_names[LOG_ERR] = "error";
    log_priority_names[LOG_WARNING] = "warning";
    log_priority_names[LOG_NOTICE] = "notice";
    log_priority_names[LOG_INFO] = "info";
    log_priority_names[LOG_DEBUG] = "debug";

    setlogmask(LOG_UPTO(__isProgram ? LOG_LOWER_PRIORITY_PROGRAM_DEFAULT : LOG_LOWER_PRIORITY_DAEMON_DEFAULT));

    isProgram = __isProgram;
    if (isProgram) flagSyslog = false;
    else {
        flagStdout = false;
        flagStderr = false;
    }

    // open log
    openlog(NULL, __option, __facility);

    // set config event
//    { struct config_event_t event =  { (const char *)LOG_LOWER_PRIORITY_DAEMON_VAR, log_slot, JSON_STRING }; config_setEvent(event); }
//    { struct config_event_t event =  { (const char *)LOG_LOWER_PRIORITY_PROGRAM_VAR, log_slot, JSON_STRING }; config_setEvent(event); }
}

void log_slot(const char* key, const json_t *value, const char* filename)
{
    // log lower daemon priority
    if (!isProgram && !strcmp(key, LOG_LOWER_PRIORITY_DAEMON_VAR)) {
        if (value == NULL) {
            // set value by default
            setlogmask(LOG_UPTO(LOG_LOWER_PRIORITY_DAEMON_DEFAULT));      // or to example: setlogmask(setlogmask(0) & ~LOG_MASK(LOG_DEBUG));
            return;
        }

        bool isExist = false;
        size_t i;
        const char *str_value = json_string_value(value);
        for (i = 0; i < 8; ++i) {
            if (!strcmp(log_priority_names[i], str_value)) {
                isExist = true;
                setlogmask(LOG_UPTO(i));
                log_print(LOG_MSG_DEBUG, "configuration file \"%s\": field: \"%s\": value is changed: \"%s\"", filename, key, str_value);
                break;
            }
        }

        if (!isExist) log_print(LOG_MSG_ERR,
                                "configuration file \"%s\": field: \"%s\": value isn't correct "
                                "(\"%s\"|\"%s\"|\"%s\"|\"%s\"|\"%s\"|\"%s\"|\"%s\"|\"%s\")",
                                filename,
                                key,
                                log_priority_names[LOG_EMERG],
                                log_priority_names[LOG_ALERT],
                                log_priority_names[LOG_CRIT],
                                log_priority_names[LOG_ERR],
                                log_priority_names[LOG_WARNING],
                                log_priority_names[LOG_NOTICE],
                                log_priority_names[LOG_INFO],
                                log_priority_names[LOG_DEBUG]);
    }
    // log lower program priority
    if (isProgram && !strcmp(key, LOG_LOWER_PRIORITY_PROGRAM_VAR)) {
        if (value == NULL) {
            // set value by default
            setlogmask(LOG_UPTO(LOG_LOWER_PRIORITY_PROGRAM_DEFAULT));      // or to example: setlogmask(setlogmask(0) & ~LOG_MASK(LOG_DEBUG));
            return;
        }

        bool isExist = false;
        size_t i;
        const char *str_value = json_string_value(value);
        for (i = 0; i < 8; ++i) {
            if (!strcmp(log_priority_names[i], str_value)) {
                isExist = true;
                setlogmask(LOG_UPTO(i));
                log_print(LOG_MSG_DEBUG, "configuration file \"%s\": field: \"%s\": value is changed: \"%s\"", filename, key, str_value);
                break;
            }
        }

        if (!isExist) log_print(LOG_MSG_ERR,
                                "configuration file \"%s\": field: \"%s\": value isn't correct "
                                "(\"%s\"|\"%s\"|\"%s\"|\"%s\"|\"%s\"|\"%s\"|\"%s\"|\"%s\")",
                                filename,
                                key,
                                log_priority_names[LOG_EMERG],
                                log_priority_names[LOG_ALERT],
                                log_priority_names[LOG_CRIT],
                                log_priority_names[LOG_ERR],
                                log_priority_names[LOG_WARNING],
                                log_priority_names[LOG_NOTICE],
                                log_priority_names[LOG_INFO],
                                log_priority_names[LOG_DEBUG]);
    }

}

void log_print(const char *filename, const char *functionname, int linenumber, int __pri, const char *__fmt, ...)
{
    _UNUSED(filename);
    _UNUSED(functionname);
    _UNUSED(linenumber);
    _UNUSED(__pri);
    _UNUSED(__fmt);
//*/

    if (log_priority_names[0] == 0) return;
    if (!(setlogmask(0) & LOG_MASK(__pri))) return;

    // Preparing a message
    // ---------------------------------------
    // add debug information
    char preprocessor_str[(filename ? strlen(filename) : 0)
            + (functionname ? strlen(functionname) : 0)
            + (linenumber > 0 ? 20 : 0)
            + 6];
    preprocessor_str[0] = '\0';

    // flag LOG_DEBUG is enabled
//    if (setlogmask(0) & LOG_MASK(LOG_DEBUG)) {
        if (filename != NULL) sprintf(preprocessor_str, "\"%s\": ", filename);
        if (functionname != NULL) sprintf(preprocessor_str + strlen(preprocessor_str),
                                          "%s:%s", functionname, ((linenumber > 0) ? "" : " " ));
        if (linenumber > 0) sprintf(preprocessor_str + strlen(preprocessor_str),
                                    "%d: ", linenumber);
//    }

    // add caption
    char caption[12] = { 0 };
    switch (__pri) {
    case LOG_EMERG:
        strcpy(caption, log_priority_names[LOG_EMERG]);
        break;
    case LOG_ALERT:
        strcpy(caption, log_priority_names[LOG_ALERT]);
        break;
    case LOG_CRIT:
        strcpy(caption, log_priority_names[LOG_CRIT]);
        break;
    case LOG_ERR:
        strcpy(caption, log_priority_names[LOG_ERR]);
        break;
    case LOG_WARNING:
        strcpy(caption, log_priority_names[LOG_WARNING]);
        break;
    case LOG_NOTICE:
        strcpy(caption, log_priority_names[LOG_NOTICE]);
        break;
    case LOG_INFO:

        break;
    case LOG_DEBUG:
        strcpy(caption, log_priority_names[LOG_DEBUG]);
        break;
    }
    if (__pri != LOG_INFO) strcat(caption, ": ");

    char format[strlen(preprocessor_str) + strlen(caption) + strlen(__fmt) + 1];
    format[0] = '\0';
    strcat(format, preprocessor_str);
    strcat(format, caption);
    strcat(format, __fmt);



    // print to log
    if (flagSyslog) {
        va_list ap;
        va_start(ap, __fmt);
        vsyslog(__pri, format, ap);
        va_end(ap);
    }


    // print to stdout/stderr
    switch (__pri) {
    case LOG_EMERG:
    case LOG_ALERT:
    case LOG_CRIT:
    case LOG_ERR:
    case LOG_WARNING:
        if (flagStderr) {
            va_list ap;
            va_start(ap, __fmt);
            vfprintf(stderr, format, ap);
            va_end(ap);
            fprintf(stderr, "\n");
        }
        break;
    case LOG_NOTICE:
    case LOG_INFO:
    case LOG_DEBUG:
        if (flagStdout) {
            va_list ap;
            va_start(ap, __fmt);
            vfprintf(stderr, format, ap);
            va_end(ap);
            fprintf(stderr, "\n");
        }
        break;
    }



    // action
    switch (__pri) {
    case LOG_EMERG:
    case LOG_ALERT:
    case LOG_CRIT:
        exit(EXIT_FAILURE);
        break;
    case LOG_ERR:

        break;
    case LOG_WARNING:

        break;
    case LOG_NOTICE:

        break;
    case LOG_INFO:

        break;
    case LOG_DEBUG:

        break;
    }
//*/

    return;
}

void log_hex_dump(const void *src, char *buffer, size_t length, size_t line_size, char *prefix)
{
    int i = 0;
    const unsigned char *address = src;
    const unsigned char *line = address;
    unsigned char c;

    sprintf(buffer, "%s | ", (prefix != NULL) ? prefix : "");
    while (length-- > 0) {
        sprintf(buffer + strlen(buffer), "%02X ", *address++);
        if (!(++i % line_size) || (length == 0 && i % line_size)) {
            if (length == 0) {
                while (i++ % line_size)
                    sprintf(buffer + strlen(buffer), "__ ");
            }
            sprintf(buffer + strlen(buffer), " | ");  /* right close */
            while (line < address) {
                c = *line++;
                sprintf(buffer + strlen(buffer), "%c", (c < 33 || c == 255) ? 0x2E : c);
            }
            if (length > 0)
                sprintf(buffer + strlen(buffer), "%s | ", prefix);
        }
    }
}
