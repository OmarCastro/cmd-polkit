#include <gtk/gtk.h>

void     mock_gtk_init                 (int    *argc,
                                   char ***argv);


GDK_AVAILABLE_IN_ALL
GtkWidget* mock_gtk_message_dialog_new      (GtkWindow      *parent,
                                        GtkDialogFlags  flags,
                                        GtkMessageType  type,
                                        GtkButtonsType  buttons,
                                        const gchar    *message_format,
                                        ...) G_GNUC_PRINTF (5, 6);


gint mock_gtk_dialog_run (GtkDialog *dialog);

void mock_gtk_widget_destroy (GtkWidget	*widget);

void mock_gtk_window_set_title (GtkWindow *window, const gchar  *title);



int  called_times_gtk_init();
int  called_times_gtk_dialog_run();
void setup_gtk_mock();

#undef  GTK_WINDOW
#define  GTK_WINDOW(window) ((GtkWindow*)(void *)window)

#undef  GTK_DIALOG
#define  GTK_DIALOG(window) ((GtkDialog*)(void *)window)

#undef  GTK_WIDGET
#define  GTK_WIDGET(window) ((GtkWidget*)(void *)window)


#define gtk_init(argc, argv) mock_gtk_init(argc, argv)
#define gtk_dialog_run(widget) mock_gtk_dialog_run(widget)
#define gtk_widget_destroy(widget) mock_gtk_widget_destroy(widget)
#define gtk_message_dialog_new(parent,flags,type,buttons,message_format, ...) mock_gtk_message_dialog_new (parent,flags,type,buttons,message_format, __VA_ARGS__)
#define gtk_window_set_title(widget,title) mock_gtk_window_set_title (widget,title)

