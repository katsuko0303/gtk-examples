#pragma once
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define	MY_TYPE_NAME_DIALOG	my_name_dialog_get_type()
G_DECLARE_FINAL_TYPE(MyNameDialog, my_name_dialog, MY, NAME_DIALOG, GtkWindow)

GtkWidget * my_name_dialog_new();
void my_name_dialog_ask(MyNameDialog *dialog, GtkWindow *parent, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer cb_data);
gchar * my_name_dialog_ask_finish(MyNameDialog *dialog, GAsyncResult *result, GError **error);

G_END_DECLS

