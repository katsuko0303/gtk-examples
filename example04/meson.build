project('example04', 'c')

libs = dependency('gtk4')

gnome = import('gnome')
resources = gnome.compile_resources('resources', 'app.gresource.xml',
  dependencies: [files('window.ui', 'address_dialog.ui')],
  source_dir: [meson.source_root()])

executable('example04', ['main.c', 'window.c', 'address_dialog.c', 'address_data.c', resources],
  dependencies: [libs])

