project('example09', 'c')

libs = dependency('gtk4')

gnome = import('gnome')
resources = gnome.compile_resources('resources', 'app.gresource.xml',
  dependencies: [files('window.ui')],
  source_dir: [meson.source_root()])

executable('example09', ['main.c', 'window.c', 'file_item.c', resources],
  dependencies: [libs])

