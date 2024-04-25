project('example07', 'c')

libs = dependency('gtk4')

gnome = import('gnome')
resources = gnome.compile_resources('resources', 'app.gresource.xml',
  dependencies: [files('window.ui')],
  source_dir: [meson.source_root()])

executable('example07', ['main.c', 'window.c', 'file_item.c', 'file_item_widget.c', 'image_loader.c', resources],
  dependencies: [libs])

