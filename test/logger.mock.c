#include <stdbool.h>
#include <glib.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>

GString * get_stdout();
GString * get_stderr();
GString *memory_stdout = NULL;
GString *memory_stderr = NULL;

static void mock_printf(const char * format, ...){

  va_list arglist;
  va_start( arglist, format );
  g_string_append_vprintf( get_stdout(), format, arglist );
  va_end( arglist );

}

static void mock_fprintf(FILE *stream, const char *format, ...){
  GString * memoryfile = stream == stderr ? get_stderr() : get_stdout();
  va_list arglist;
  va_start( arglist, format );
  g_string_append_vprintf( memoryfile, format, arglist );
  va_end( arglist );

}

static void mock_vprintf(const char *format, va_list arglist){
  g_string_append_vprintf( get_stdout(), format, arglist );
}


// mock
#define printf(f_, ...) mock_printf((f_), ##__VA_ARGS__)
#define fprintf(s_, f_, ...) mock_fprintf((s_),(f_), ##__VA_ARGS__)
#define vprintf(f_, a_) mock_vprintf((f_), (a_))


#include "../src/cmdline.c"
#include "../src/logger.c"
#include "logger.mock.h"

GString * get_stdout(){
    if(memory_stdout == NULL){
		memory_stdout = g_string_new("");
	}
    return memory_stdout;
}

GString * get_stderr(){
    if(memory_stderr == NULL){
		memory_stderr = g_string_new("");
	}
    return memory_stderr;
}


void reset_logs() {
    g_string_truncate(get_stdout(), 0);
    g_string_truncate(get_stderr(), 0);
    silenced_logs = false;
    verbose_logs = false;
}



