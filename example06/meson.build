project('example06', 'c')

libs = dependency('gtk4')

gnome = import('gnome')
resources = gnome.compile_resources('resources', 'app.gresource.xml',
  dependencies: [files('window.ui', 'line_drawer.ui')],
  source_dir: [meson.source_root()])

executable('example06',
  ['main.c', 'window.c', 'paintable.c', 'drawer.c', 'line_drawer.c', 'rect_area_drawer.c', 'rect_drawer.c', 'erase_drawer.c', resources],
  dependencies: [libs])

