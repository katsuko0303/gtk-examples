# coding: utf-8

import sys, os, traceback
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gdk, GLib, Gio, GdkPixbuf

ICON_SIZE = 128

#----------------------------------------------------------------------
#	image_loader

class ImageLoader(GObject.GObject):
	__gtype_name__ = 'MyImageLoader'

	@GObject.Property(type=Gio.File, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.CONSTRUCT_ONLY))
	def file(self):
		return self._file if hasattr(self, '_file') else None
	@file.setter
	def file(self, value):
		self._file = value

	def __init__(self, **kwargs):
		self._scaling = None
		super().__init__(**kwargs)

	def load_async(self, cancellable, callback, *args):
		self._scaling = None
		task = Gio.Task.new(self, cancellable, callback, *args)
		task.run_in_thread(self._thread_func)

	def load_at_scale_async(self, width, height, cancellable, callback, *args):
		self._scaling = width, height
		task = Gio.Task.new(self, cancellable, callback, *args)
		task.run_in_thread(self._thread_func)

	def load_finish(self, result):
		ret, obj = result.propagate_value()
		return obj if ret else None

	@staticmethod
	def _thread_func(task, source, task_data, cancellable):
		try:
			stream = source.file.read(cancellable)
			if source._scaling:
				width, height = source._scaling
				pixbuf = GdkPixbuf.Pixbuf.new_from_stream_at_scale(stream, width, height, True, cancellable)
			else:
				pixbuf = GdkPixbuf.Pixbuf.new_from_stream(stream, cancellable)
			texture = Gdk.Texture.new_for_pixbuf(pixbuf)
			value = GObject.Value(value_type=Gdk.Texture)
			value.set_object(texture)
			task.return_value(value)
		except GLib.Error as e:
			task.return_error(e)

#----------------------------------------------------------------------
#	file_item

class FileItem(GObject.GObject):
	__gtype_name__ = 'MyFileItem'

	FILE_ATTRIBUTES = ','.join([
		Gio.FILE_ATTRIBUTE_STANDARD_NAME,
		Gio.FILE_ATTRIBUTE_STANDARD_ICON,
		Gio.FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
		Gio.FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
		Gio.FILE_ATTRIBUTE_STANDARD_TYPE,
		])

	@GObject.Property(type=Gio.File, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.CONSTRUCT_ONLY))
	def file(self):
		return self._file if hasattr(self, '_file') else None
	@file.setter
	def file(self, value):
		self._file = value

	@GObject.Property(type=Gio.FileInfo, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.CONSTRUCT_ONLY))
	def file_info(self):
		return self._file_info if hasattr(self, '_file_info') else None
	@file_info.setter
	def file_info(self, value):
		self._file_info = value
		self.notify('icon')
		self.notify('display-name')

	@GObject.Property(type=Gdk.Paintable)
	def icon(self):
		if self._icon_texture is not None:
			return self._icon_texture
		if self.file_info is not None:
			icon = self.file_info.get_icon()
			if icon is not None:
				return Gtk.IconTheme.get_for_display(Gdk.Display.get_default()).lookup_by_gicon(icon, ICON_SIZE, 1, Gtk.TextDirection.NONE, 0)
		return None

	@GObject.Property(type=GObject.TYPE_STRING)
	def display_name(self):
		return self.file_info.get_display_name() if self.file_info is not None else ''

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=False)
	def is_directory(self):
		return self.file_info is not None and self.file_info.get_file_type() == Gio.FileType.DIRECTORY

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=False)
	def is_image_file(self):
		if self.file_info is None:
			return False
		content_type = self.file_info.get_content_type()
		return content_type and Gio.content_type_get_mime_type(content_type).startswith('image/')

	def __init__(self, **kwargs):
		self._icon_texture = None
		self._loader = None
		super().__init__(**kwargs)

	def load_icon_texture(self):
		if self._icon_texture is not None:
			return
		if self._loader is not None:
			return
		if self.file_info is None or not self.is_image_file:
			return
		self._loader = ImageLoader(file=self.file)
		self._loader.load_at_scale_async(ICON_SIZE, ICON_SIZE, None, self._on_loaded_icon_texture)

	def _on_loaded_icon_texture(self, source, result):
		try:
			self._icon_texture = source.load_finish(result)
			self.notify('icon')
		except GLib.Error as e:
			traceback.print_exc()
		self._loader = None

#----------------------------------------------------------------------
#	file_item_widget

class FileItemWidget(Gtk.Box):
	__gtype_name__ = 'MyFileItemWidget'

	@GObject.Property(type=FileItem)
	def item(self):
		return self._item if hasattr(self, '_item') else None
	@item.setter
	def item(self, value):
		self._item = value
		if self._item is not None:
			if self._label is not None:
				self._label.set_label(self._item.display_name)
				self._item.bind_property('display-name', self._label, 'label', GObject.BindingFlags.DEFAULT)
			if self._image is not None:
				self._image.set_from_paintable(self._item.icon)
				self._item.bind_property('icon', self._image, 'paintable', GObject.BindingFlags.DEFAULT)
				if self._image.get_mapped():
					self._item.load_icon_texture()

	def __init__(self, **kwargs):
		self._image = None
		self._label = None
		super().__init__(orientation=Gtk.Orientation.VERTICAL, spacing=4, **kwargs)

		self._image = Gtk.Image.new_from_paintable(self.item.icon if self.item is not None else None)
		self._image.set_size_request(ICON_SIZE, ICON_SIZE)
		self._image.connect('map', self._on_image_map)
		if self.item is not None:
			self.item.bind_property('icon', self._image, 'paintable', GObject.BindingFlags.DEFAULT)
		self.append(self._image)

		self._label = Gtk.Label(label=(self.item.display_name if self.item is not None else ''))
		if self.item is not None:
			self.item.bind_property('display-name', self._label, 'label', GObject.BindingFlags.DEFAULT)
		self.append(self._label)

	def _on_image_map(self, widget):
		if self.item is not None:
			self.item.load_icon_texture()

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=Gio.File)
	def current_folder(self):
		return self._current_folder if hasattr(self, '_current_folder') else None
	@current_folder.setter
	def current_folder(self, value):
		self._current_folder = value

		if self._task is not None:
			self._task.get_cancellable().cancel()
		self._task = Gio.Task.new(self, Gio.Cancellable(), self._on_loaded_folder)
		folder = self._current_folder
		self._task.run_in_thread(lambda task, source, data, cancellable: self._load_folder_thread_func(task, source, data, cancellable, folder))

	@GObject.Property(type=Gio.ListModel)
	def file_model(self):
		return self._file_model

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=False)
	def image_active(self):
		return self._image_active

	@GObject.Property(type=Gdk.Paintable)
	def image_paintable(self):
		return self._image_paintable

	def __init__(self, **kwargs):
		self._task = None
		self._file_model = None
		self._image_active = False
		self._image_paintable = None
		super().__init__(**kwargs)
		self.current_folder = Gio.File.new_for_path('.')

	@staticmethod
	def _load_folder_thread_func(task, source, task_data, cancellable, folder):
		try:
			enumerator = folder.enumerate_children(FileItem.FILE_ATTRIBUTES, Gio.FileQueryInfoFlags.NONE, cancellable)
			try:
				list_store = Gio.ListStore(item_type=FileItem)
				while True:
					file_info = enumerator.next_file(cancellable)
					if file_info is None:
						break
					child = enumerator.get_child(file_info)
					item = FileItem(file=child, file_info=file_info)
					list_store.append(item)
			finally:
				enumerator.close()
			value = GObject.Value(value_type=Gio.ListStore)
			value.set_object(list_store)
			task.return_value(value)
		except GLib.Error as e:
			task.return_error(e)

	def _on_loaded_folder(self, source, result):
		try:
			ret, obj = result.propagate_value()
			self._file_model = obj if ret else None
			self.notify('file-model')
		except GLib.Error as e:
			if not e.matches(Gio.io_error_quark(), Gio.IOErrorEnum.CANCELLED):
				traceback.print_exc()
				self._task = None

	@Gtk.Template.Callback('on_go_up')
	def _on_go_up(self, button):
		parent = self.current_folder.get_parent()
		if parent is not None:
			self.current_folder = parent

	@Gtk.Template.Callback('on_file_list_activate')
	def _on_file_list_activate(self, grid_view, position):
		if position != Gtk.INVALID_LIST_POSITION:
			item = grid_view.get_model().get_item(position)
			if item.is_directory:
				self.current_folder = item.file
			elif item.is_image_file:
				self._image_active = True
				self.notify('image-active')

				loader = ImageLoader(file=item.file)
				loader.load_async(None, self._on_load_image)

	def _on_load_image(self, source, result):
		try:
			self._image_paintable = source.load_finish(result)
			self.notify('image-paintable')
		except GLib.Error as e:
			traceback.print_exc()

	@Gtk.Template.Callback('on_close_image')
	def _on_close_image(self, button):
		self._image_active = False
		self.notify('image-active')

#----------------------------------------------------------------------
#	main

def on_startup(app):
	provider = Gtk.CssProvider()
	provider.load_from_path(os.path.join(os.path.dirname(__file__), 'app.css'))
	Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(), provider, Gtk.STYLE_PROVIDER_PRIORITY_FALLBACK);

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('startup', on_startup)
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

