#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <stdbool.h>
#include "app.h"

static bool is_gtk_initialized = false;

void lazy_init_gtk(){
    if(!is_gtk_initialized){
        Application app = app_get();
          gtk_init(&app.argc, &app.argv);
        is_gtk_initialized = true;
    }
}

void show_error_message(const char * const text){

    lazy_init_gtk();
	GtkWidget * dialog = gtk_message_dialog_new (NULL,
                                 0,
                                 GTK_MESSAGE_ERROR,
                                 GTK_BUTTONS_CLOSE,
                                 "%s",
                                 text
                                 );        
        gtk_window_set_title(GTK_WINDOW(dialog), "cmd polkit agent");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy( GTK_WIDGET(dialog) );
}

void show_error_message_format(const char * const format, ...){
    lazy_init_gtk();

  	char * result;
  	va_list arglist;
	va_start( arglist, format );
	vasprintf( &result, format, arglist );
	va_end( arglist );

	GtkWidget * dialog = gtk_message_dialog_new (NULL,
                                 0,
                                 GTK_MESSAGE_ERROR,
                                 GTK_BUTTONS_CLOSE,
                                 "%s",
                                 result
                                 );        
        gtk_window_set_title(GTK_WINDOW(dialog), "cmd polkit agent");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy( GTK_WIDGET(dialog) );
        free(result);
}
