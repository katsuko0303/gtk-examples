project('example01', 'c')
gnome = import('gnome')
libs = dependency('gtk4')
resources = gnome.compile_resources('resources', 'app.gresource.xml',
  dependencies: files('window.ui', 'page.ui'),
  source_dir: meson.source_root())
executable(
  'example01', ['main.c', 'application.c', 'window.c', resources],
  dependencies: [libs])

