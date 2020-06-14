#ifndef APP_H
#define APP_H
#include <stdbool.h>

typedef enum {
   AuthHandlingMode_SERIE,
   AuthHandlingMode_PARALLEL
} AuthHandlingMode;

const char*  app__get_cmd_line();
AuthHandlingMode app__get_auth_handling_mode();

#endif

