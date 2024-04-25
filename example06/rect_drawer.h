#pragma once
#include "rect_area_drawer.h"

G_BEGIN_DECLS

#define	MY_TYPE_RECT_DRAWER	my_rect_drawer_get_type()
G_DECLARE_FINAL_TYPE(MyRectDrawer, my_rect_drawer, MY, RECT_DRAWER, MyRectAreaDrawer)

MyDrawer * my_rect_drawer_new();

G_END_DECLS


