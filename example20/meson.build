project('example20', 'c')

libs = dependency('gtk4')

gnome = import('gnome')
resources = gnome.compile_resources('resources', 'app.gresource.xml',
  dependencies: [files('window.ui')],
  source_dir: [meson.source_root()])

executable('example20', ['main.c', 'window.c', 'no_ui_window.c', 'list_item.c', resources],
  dependencies: [libs])

