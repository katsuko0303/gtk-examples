project('example02', 'c')
gnome = import('gnome')
libs = dependency('gtk4')
resources = gnome.compile_resources('resources', 'app.gresource.xml',
  dependencies: files('window.ui'),
  source_dir: meson.source_root())
executable(
  'example02', ['main.c', 'window.c', resources],
  dependencies: [libs])

