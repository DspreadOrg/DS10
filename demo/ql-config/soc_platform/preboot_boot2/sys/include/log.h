#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <inttypes.h>
#include <log-conf.h>
#include "clock.h"

#ifndef MIN
#define MIN(n, m)   (((n) < (m)) ? (n) : (m))
#endif

/* The different log levels available */
#define LOG_LEVEL_NONE         0 /* No log */
#define LOG_LEVEL_ERR          1 /* Errors */
#define LOG_LEVEL_WARN         2 /* Warnings */
#define LOG_LEVEL_PRINT        3 /* Print */
#define LOG_LEVEL_INFO         4 /* Basic info */
#define LOG_LEVEL_DBG          5 /* Detailled debug */

/* Per-module log level */

struct log_module {
  const char *name;
  int *curr_log_level;
  int max_log_level;
};

extern int curr_log_level_main;

extern struct log_module all_modules[];

#define LOG_LEVEL_MAIN                        MIN((LOG_CONF_LEVEL_MAIN), curr_log_level_main)

/* Main log function */

#define LOG(newline, level, levelstr, ...) do {  \
                            if(level <= (LOG_LEVEL)) { \
                              if(newline) { \
                                if(LOG_WITH_TIME_PREFIX) { \
                                  LOG_OUTPUT("[%10" PRIu32 "] ", clock_time()); \
                                } \
                                if(LOG_WITH_MODULE_PREFIX) { \
                                  LOG_OUTPUT_PREFIX(level, levelstr, LOG_MODULE); \
                                } \
                                if(LOG_WITH_LOC) { \
                                  LOG_OUTPUT("[%s: %d] ", __FILE__, __LINE__); \
                                } \
                              } \
                              LOG_OUTPUT(__VA_ARGS__); \
                            } \
                          } while (0)

/* More compact versions of LOG macros */
#define LOG_ERR(...)           LOG(1, LOG_LEVEL_ERR, "ERR", __VA_ARGS__)
#define LOG_WARN(...)          LOG(1, LOG_LEVEL_WARN, "WARN", __VA_ARGS__)
#define LOG_PRINT(...)         LOG(1, LOG_LEVEL_PRINT, "PRI", __VA_ARGS__)
#define LOG_INFO(...)          LOG(1, LOG_LEVEL_INFO, "INFO", __VA_ARGS__)
#define LOG_DBG(...)           LOG(1, LOG_LEVEL_DBG, "DBG", __VA_ARGS__)

#define LOG_ERR_(...)           LOG(0, LOG_LEVEL_ERR, "ERR", __VA_ARGS__)
#define LOG_WARN_(...)          LOG(0, LOG_LEVEL_WARN, "WARN", __VA_ARGS__)
#define LOG_PRINT_(...)         LOG(0, LOG_LEVEL_PRINT, "PRI", __VA_ARGS__)
#define LOG_INFO_(...)          LOG(0, LOG_LEVEL_INFO, "INFO", __VA_ARGS__)
#define LOG_DBG_(...)           LOG(0, LOG_LEVEL_DBG, "DBG", __VA_ARGS__)

/* For checking log level.
   As this builds on curr_log_level variables, this should not be used
   in pre-processor macros. Use in a C 'if' statement instead, e.g.:
   if(LOG_INFO_ENABLED) { ... }
   Note that most compilers will still be able to strip the code out
   for low enough log levels configurations. */
#define LOG_ERR_ENABLED        ((LOG_LEVEL) >= LOG_LEVEL_ERR)
#define LOG_WARN_ENABLED       ((LOG_LEVEL) >= LOG_LEVEL_WARN)
#define LOG_INFO_ENABLED       ((LOG_LEVEL) >= LOG_LEVEL_INFO)
#define LOG_DBG_ENABLED        ((LOG_LEVEL) >= LOG_LEVEL_DBG)

/**
 * Sets a log level at run-time. Logs are included in the firmware via
 * the compile-time flags in log-conf.h, but this allows to force lower log
 * levels, system-wide.
 * \param module The target module string descriptor
 * \param level The log level
*/
void log_set_level(const char *module, int level);

/**
 * Returns the current log level.
 * \param module The target module string descriptor
 * \return The current log level
*/
int log_get_level(const char *module);

/**
 * Returns a textual description of a log level
 * \param level log level
 * \return The textual description
*/
const char *log_level_to_str(int level);

#endif /* __LOG_H__ */

/** @} */
/** @} */
