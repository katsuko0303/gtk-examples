project('example11', 'c')

libs = dependency('gtk4')

gnome = import('gnome')
resources = gnome.compile_resources('resources', 'app.gresource.xml',
  dependencies: [files('name_dialog.ui')],
  source_dir: [meson.source_root()])

executable('example11', ['main.c', 'window.c', 'name_dialog.c', resources],
  dependencies: [libs])

