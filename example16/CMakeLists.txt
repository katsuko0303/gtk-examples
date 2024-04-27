CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(example16)

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(LIBS REQUIRED gtk4)

INCLUDE_DIRECTORIES(${LIBS_INCLUDE_DIRS})

ADD_CUSTOM_COMMAND(
	OUTPUT resource.c
	COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && glib-compile-resources app.gresource.xml --target ${CMAKE_CURRENT_BINARY_DIR}/resource.c --generate-source
	DEPENDS
		app.gresource.xml
		window.ui
		app.css
)

ADD_EXECUTABLE(example16
	main.c
	window.c
	resource.c
)
TARGET_LINK_LIBRARIES(example16 ${LIBS_LIBRARIES})

