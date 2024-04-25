CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(example08)

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(LIB REQUIRED gtk4)

INCLUDE_DIRECTORIES(${LIB_INCLUDE_DIRS})

ADD_CUSTOM_COMMAND(
	OUTPUT resource.c
	COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && glib-compile-resources app.gresource.xml --target ${CMAKE_CURRENT_BINARY_DIR}/resource.c --generate-source
	DEPENDS
		app.gresource.xml
		window.ui
)

ADD_EXECUTABLE(example08
	main.c
	window.c
	resource.c
)
TARGET_LINK_LIBRARIES(example08 ${LIB_LIBRARIES})

