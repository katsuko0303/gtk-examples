#pragma once
#include "rect_area_drawer.h"

G_BEGIN_DECLS

#define	MY_TYPE_ERASE_DRAWER	my_erase_drawer_get_type()
G_DECLARE_FINAL_TYPE(MyEraseDrawer, my_erase_drawer, MY, ERASE_DRAWER, MyRectAreaDrawer)

MyDrawer * my_erase_drawer_new();

G_END_DECLS


