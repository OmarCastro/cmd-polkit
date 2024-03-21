#include "error-message.mock.h"
#include "../src/error-message.dialog.c"
#include <stdbool.h>

void reset_lazy_init_gtk(){
    is_gtk_initialized = false;
}


