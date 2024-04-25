#pragma once
#include <glib-object.h>

G_BEGIN_DECLS

#define	MY_TYPE_ADDRESS_DATA	my_address_data_get_type()
G_DECLARE_FINAL_TYPE(MyAddressData, my_address_data, MY, ADDRESS_DATA, GObject)

MyAddressData * my_address_data_new();
const gchar * my_address_data_get_first_name(MyAddressData *address_data);
void my_address_data_set_first_name(MyAddressData *address_data, const gchar *first_name);
const gchar * my_address_data_get_last_name(MyAddressData *address_data);
void my_address_data_set_last_name(MyAddressData *address_data, const gchar *last_name);
const GDate * my_address_data_get_birth_day(MyAddressData *address_data);
void my_address_data_set_birth_day(MyAddressData *address_data, const GDate *birth_day);
gint my_address_data_get_age(MyAddressData *address_data);
#if 0
void my_address_data_set_age(MyAddressData *address_data, gint age);
#endif
const gchar * my_address_data_get_address(MyAddressData *address_data);
void my_address_data_set_address(MyAddressData *address_data, const gchar *address);
const gchar * my_address_data_get_email(MyAddressData *address_data);
void my_address_data_set_email(MyAddressData *address_data, const gchar *email);

G_END_DECLS


