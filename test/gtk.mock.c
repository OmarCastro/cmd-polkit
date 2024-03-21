#include "gtk.mock.h"

static int callTimesGtkInit = 0;
static int callTimesGtkDialogRun = 0;

void   mock_gtk_init (int  *argc, char ***argv){
    callTimesGtkInit++;
}

int  called_times_gtk_init(){
    return callTimesGtkInit;
}



void setup_gtk_mock(){
    callTimesGtkInit = 0;
}

gint mock_gtk_dialog_run (GtkDialog *dialog){
    callTimesGtkDialogRun++;
    return GTK_RESPONSE_NONE;
}

int  called_times_gtk_dialog_run(){
    return callTimesGtkDialogRun;
}

GtkWidget* mock_gtk_message_dialog_new      (GtkWindow      *parent,
                                        GtkDialogFlags  flags,
                                        GtkMessageType  type,
                                        GtkButtonsType  buttons,
                                        const gchar    *message_format,
                                        ...) {
    // Mock implementation: return a dummy GtkWidget pointer
    return NULL;
}

void mock_gtk_widget_destroy		  (GtkWidget	       *widget){
    // Mock implementation: do nothing
}


void mock_gtk_window_set_title (GtkWindow *window, const gchar  *title){
    // Mock implementation: do nothing
}
