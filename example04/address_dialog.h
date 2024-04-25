#pragma once
#include <gtk/gtk.h>
#include "address_data.h"

G_BEGIN_DECLS

#define	MY_TYPE_ADDRESS_DIALOG	my_address_dialog_get_type()
G_DECLARE_FINAL_TYPE(MyAddressDialog, my_address_dialog, MY, ADDRESS_DIALOG, GtkWindow)

GtkWidget * my_address_dialog_new();
void my_address_dialog_edit_async(MyAddressDialog *dialog, GtkWindow *parent, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer cb_data);
MyAddressData * my_address_dialog_edit_finish(MyAddressDialog *dialog, GAsyncResult *result, GError **error);
MyAddressData * my_address_dialog_get_target(MyAddressDialog *dialog);
void my_address_dialog_set_target(MyAddressDialog *dialog, MyAddressData *target);

G_END_DECLS


