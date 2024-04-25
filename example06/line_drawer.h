#pragma once
#include "drawer.h"

G_BEGIN_DECLS

#define	MY_TYPE_LINE_DRAWER	my_line_drawer_get_type()
G_DECLARE_FINAL_TYPE(MyLineDrawer, my_line_drawer, MY, LINE_DRAWER, MyDrawer)

MyDrawer * my_line_drawer_new();

G_END_DECLS

