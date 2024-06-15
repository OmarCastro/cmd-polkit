#include "gtk.mock.h"

static int callTimesGtkInit = 0;
static int callTimesGtkDialogRun = 0;

void   mock_gtk_init ([[maybe_unused]] int  *argc, [[maybe_unused]] char ***argv){
    callTimesGtkInit++;
}

int  called_times_gtk_init(){
    return callTimesGtkInit;
}



void setup_gtk_mock(){
    callTimesGtkInit = 0;
}

gint mock_gtk_dialog_run ([[maybe_unused]] GtkDialog *dialog){
    callTimesGtkDialogRun++;
    return GTK_RESPONSE_NONE;
}

int  called_times_gtk_dialog_run(){
    return callTimesGtkDialogRun;
}

GtkWidget* mock_gtk_message_dialog_new ([[maybe_unused]] GtkWindow      *parent,
                                        [[maybe_unused]] GtkDialogFlags  flags,
                                        [[maybe_unused]] GtkMessageType  type,
                                        [[maybe_unused]] GtkButtonsType  buttons,
                                        [[maybe_unused]] const gchar    *message_format,
                                        ...) {
    // Mock implementation: return a dummy GtkWidget pointer
    return NULL;
}

void mock_gtk_widget_destroy		  ([[maybe_unused]] GtkWidget *widget){
    // Mock implementation: do nothing
}


void mock_gtk_window_set_title ([[maybe_unused]] GtkWindow *window, [[maybe_unused]] const gchar  *title){
    // Mock implementation: do nothing
}
