# coding: utf-8

import sys, os, traceback
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gio, GLib

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window, Gtk.BuilderScope):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=Gio.File)
	def top_folder(self):
		return self._top_folder if hasattr(self, '_top_folder') else Gio.File.new_for_path(GLib.get_home_dir())
	@top_folder.setter
	def top_folder(self, value):
		self._top_folder = value

		self._enumerate_files_recursively([self._top_folder])
		self.loading = True

	@GObject.Property(type=GObject.TYPE_STRING, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def search_text(self):
		return self._search_text if hasattr(self, '_search_text') else ''
	@search_text.setter
	def search_text(self, value):
		if not hasattr(self, '_search_text') or value != self._search_text:
			self._search_text = value
			self.notify('search-text')

			self._filter.changed(Gtk.FilterChange.DIFFERENT)

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=False)
	def loading(self):
		return self._loading if hasattr(self, '_loading') else False
	@loading.setter
	def loading(self, value):
		self._loading = value
		if self._loading:
			self._spinner.start()
		else:
			self._spinner.stop()

	_file_model = Gtk.Template.Child('file_model')
	_list_view = Gtk.Template.Child('list_view')
	_filter = Gtk.Template.Child('filter')
	_spinner = Gtk.Template.Child('spinner')

	def __init__(self, **kwargs):
		super().__init__(**kwargs)

		self._filter.set_filter_func(self._filter_func)

		with open(os.path.join(os.path.dirname(__file__), 'list_item.ui'), 'rb') as fp:
			data = GLib.Bytes.new(fp.read())
		self._list_view.set_factory(Gtk.BuilderListItemFactory.new_from_bytes(self, data))

	def _filter_func(self, item):
		return len(self.search_text) == 0 or self.search_text in item.get_display_name()

	def _enumerate_files_recursively(self, folder_list):
		folder = folder_list.pop(0)
		folder.enumerate_children_async(','.join([Gio.FILE_ATTRIBUTE_STANDARD_ICON, Gio.FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
					Gio.FILE_ATTRIBUTE_STANDARD_TYPE, Gio.FILE_ATTRIBUTE_STANDARD_NAME]),
					Gio.FileQueryInfoFlags.NONE, GLib.PRIORITY_DEFAULT, None, self._on_enumerate_children, folder_list)

	def _on_enumerate_children(self, source, result, folder_list):
		try:
			enumerator = source.enumerate_children_finish(result)
		except GLib.Error as e:
			traceback.print_exc()
			if len(folder_list) > 0:
				self._enumerate_files_recursively(folder_list)
			else:
				self.loading = False
			return
		enumerator.next_files_async(32, GLib.PRIORITY_DEFAULT, None, self._on_next_files, folder_list)

	def _on_next_files(self, source, result, folder_list):
		try:
			file_infos = source.next_files_finish(result)
		except GLib.Error as e:
			traceback.print_exc()
			source.close(None)
			if len(folder_list) > 0:
				self._enumerate_files_recursively(folder_list)
			else:
				self.loading = False
			return
		if not file_infos:
			source.close(None)
			if len(folder_list) > 0:
				self._enumerate_files_recursively(folder_list)
			else:
				self.loading = False
		else:
			for file_info in file_infos:
				self._file_model.append(file_info)
				if file_info.get_file_type() == Gio.FileType.DIRECTORY:
					folder_list += [source.get_child(file_info)]
			source.next_files_async(32, GLib.PRIORITY_DEFAULT, None, self._on_next_files, folder_list)

	def do_create_closure(self, builder, function_name, flags, obj):
		return getattr(self, function_name)

	def get_icon(self, list_item, item):
		return item.get_icon() if item is not None else None

	def get_display_name(self, list_item, item):
		return item.get_display_name() if item is not None else ''

#----------------------------------------------------------------------
#	main

def on_command_line(app, command_line):
	args = command_line.get_arguments()
	if len(args) >= 2:
		top_folder = Gio.File.new_for_path(args[1])
	else:
		top_folder = Gio.File.new_for_path(GLib.get_home_dir())
	window = Window(application=app, top_folder=top_folder)
	window.set_visible(True)
	return 0

if __name__ == '__main__':
	app = Gtk.Application(flags=Gio.ApplicationFlags.HANDLES_COMMAND_LINE)
	app.connect('command-line', on_command_line)
	sys.exit(app.run(sys.argv))

