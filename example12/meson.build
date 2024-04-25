project('example12', 'c')

libs = dependency('gtk4')

gnome = import('gnome')
resources = gnome.compile_resources('resources', 'app.gresource.xml',
  dependencies: [files('window.ui', 'list_item.ui')],
  source_dir: [meson.source_root()])

executable('example12', ['main.c', 'window.c', resources],
  dependencies: [libs])

