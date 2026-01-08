#include <string.h>
#include "log.h"

int curr_log_level_main = LOG_CONF_LEVEL_MAIN;

struct log_module all_modules[] = {
  {"main", &curr_log_level_main, LOG_CONF_LEVEL_MAIN},
  {NULL, NULL, 0},
}; 
/*---------------------------------------------------------------------------*/
void
log_set_level(const char *module, int level)
{
  if(level >= LOG_LEVEL_NONE && level <= LOG_LEVEL_DBG) {
    int i = 0;
    int module_all = !strcmp("all", module);
    while(all_modules[i].name != NULL) {
      if(module_all || !strcmp(module, all_modules[i].name)) {
        *all_modules[i].curr_log_level = MIN(level, all_modules[i].max_log_level);
      }
      i++;
    }
  }
}
/*---------------------------------------------------------------------------*/
int
log_get_level(const char *module)
{
  int i = 0;
  if(module == NULL) {
    return -1;
  }
  while(all_modules[i].name != NULL) {
    if(!strcmp(module, all_modules[i].name)) {
      return *all_modules[i].curr_log_level;
    }
    i++;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
const char *
log_level_to_str(int level)
{
  switch(level) {
    case LOG_LEVEL_NONE:
      return "None";
    case LOG_LEVEL_ERR:
      return "Errors";
    case LOG_LEVEL_WARN:
      return "Warnings";
    case LOG_LEVEL_INFO:
      return "Info";
    case LOG_LEVEL_DBG:
      return "Debug";
    default:
      return "N/A";
  }
}
/** @} */
/** @} */
