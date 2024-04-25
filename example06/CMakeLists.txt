CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(example06)

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(LIB REQUIRED gtk4)

INCLUDE_DIRECTORIES(${LIB_INCLUDE_DIRS})

ADD_CUSTOM_COMMAND(
	OUTPUT resource.c
	COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && glib-compile-resources app.gresource.xml --target ${CMAKE_CURRENT_BINARY_DIR}/resource.c --generate-source
	DEPENDS
		app.gresource.xml
		window.ui
		line_drawer.ui
)

ADD_EXECUTABLE(example06
	main.c
	window.c
	paintable.c
	drawer.c
	line_drawer.c
	rect_area_drawer.c
	rect_drawer.c
	erase_drawer.c
	resource.c
)
TARGET_LINK_LIBRARIES(example06 ${LIB_LIBRARIES})

