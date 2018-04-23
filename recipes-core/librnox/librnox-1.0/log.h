#ifndef LOG_H
#define LOG_H

#include <syslog.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/* <current_file_name, current_function_name, current_line_number> */
#define LOG_PREPROCESSOR_MACROS __FILE__, __FUNCTION__, __LINE__

#define	LOG_MSG_EMERG	LOG_PREPROCESSOR_MACROS, LOG_EMERG      /* system is unusable */
#define	LOG_MSG_ALERT	LOG_PREPROCESSOR_MACROS, LOG_ALERT      /* action must be taken immediately */
#define	LOG_MSG_CRIT	LOG_PREPROCESSOR_MACROS, LOG_CRIT       /* critical conditions */
#define	LOG_MSG_ERR		LOG_PREPROCESSOR_MACROS, LOG_ERR        /* error conditions */
#define	LOG_MSG_WARNING	LOG_PREPROCESSOR_MACROS, LOG_WARNING	/* warning conditions */
#define	LOG_MSG_NOTICE	LOG_PREPROCESSOR_MACROS, LOG_NOTICE     /* normal but significant condition */
#define	LOG_MSG_INFO	LOG_PREPROCESSOR_MACROS, LOG_INFO       /* informational */
#define	LOG_MSG_DEBUG	LOG_PREPROCESSOR_MACROS, LOG_DEBUG      /* debug-level messages */

/* priority, top-down derivation
 * ------------------------------
 * LOG_EMERG
 * LOG_ALERT
 * LOG_CRIT
 * LOG_ERR
 * LOG_WARNING
 * LOG_NOTICE
 * LOG_INFO
 * LOG_DEBUG
 */

/* open connection to system logger
 *
 * wrapper for openlog function: void openlog (const char *__ident, int __option, int __facility);
 *
 * inputs:
 *      __option        i       option
 *      __facility      i       facility
 *      __isProgram      i      if 'true' - is not daemon
 *                                          по умолчанию включает вывод в консоль
 *                              if 'false' - is daemon
 *                                          по умолчанию отключает вывод в консоль
 *
 * returns:
 *      void */
void log_open(int __option, int __facility, bool __isProgram);


/* wrapper for syslog function: void syslog(int __pri, const char *__fmt);
 *
 * inputs:
 *      filename        i       current file name
 *      functionname    i       current function name
 *      linenumber      i       current line number
 *      __pri           i       priority
 *      __fmt           i       string formant
 *
 * returns:
 *      void */
void log_print(const char *filename, const char *functionname, int linenumber, int __pri, const char *__fmt, ...)
    __attribute__ ((__format__ (__printf__, 5, 6)));


/* wrapper for syslog function: void syslog(int __pri, const char *__fmt);
 *
 * inputs:
 *      src             i       source
 *      buffer          o       string value
 *      length          i       length
 *      lin_size        i       width
 *      prefix          i       prefix string
 *
 * returns:
 *      void */
void log_hex_dump(const void *src, char *buffer, size_t length, size_t line_size, char *prefix);


#endif // LOG_H
