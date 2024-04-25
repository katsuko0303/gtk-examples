project('example03', 'c')

libs = dependency('gtk4')

gnome = import('gnome')
resources = gnome.compile_resources('resources', 'app.gresource.xml',
  dependencies: [files('window.ui')],
  source_dir: [meson.source_root()])

executable('example03', ['main.c', 'window.c', 'time_paintable.c', resources],
  dependencies: [libs])

