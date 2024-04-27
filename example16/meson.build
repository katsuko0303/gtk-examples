project('example16', 'c')

libs = dependency('gtk4')

gnome = import('gnome')
resources = gnome.compile_resources('resources', 'app.gresource.xml',
  dependencies: [files('window.ui', 'app.css')],
  source_dir: [meson.source_root()])

executable('example16', ['main.c', 'window.c', resources],
  dependencies: [libs])

