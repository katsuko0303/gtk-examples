# coding: utf-8

import sys, os
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, Gio, GObject, Gdk, GdkPixbuf

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=True, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def can_shrink(self):
		return self._can_shrink if hasattr(self, '_can_shrink') else True
	@can_shrink.setter
	def can_shrink(self, value):
		if not hasattr(self, '_can_shrink') or value != self._can_shrink:
			self._can_shrink = value
			self.notify('can-shrink')

	@GObject.Property(type=Gio.ListModel)
	def content_fit_model(self):
		return self._content_fit_model

	@GObject.Property(type=GObject.TYPE_UINT, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def content_fit_selected(self):
		for i in range(self._content_fit_model.get_n_items()):
			if self._content_fit_model.get_item(i).get_string() == self.content_fit.value_nick:
				return i
		return Gtk.INVALID_LIST_POSITION
	@content_fit_selected.setter
	def content_fit_selected(self, value):
		nick = self._content_fit_model.get_item(value).get_string()
		if nick != self.content_fit.value_nick:
			for key, value in Gtk.ContentFit.__enum_values__.items():
				if value.value_nick == nick:
					self.content_fit = value
					break
			self.notify('content-fit-selected')
			self.notify('content-fit')

	@GObject.Property(type=Gtk.ContentFit, default=Gtk.ContentFit.CONTAIN, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def content_fit(self):
		return self._content_fit if hasattr(self, '_content_fit') else Gtk.ContentFit.CONTAIN
	@content_fit.setter
	def content_fit(self, value):
		if not hasattr(self, '_content_fit') or value != self._content_fit:
			self._content_fit = value
			self.notify('content-fit')
			self.notify('content-fit-selected')

	@GObject.Property(type=Gdk.Paintable)
	def paintable(self):
		return self._paintable

	def __init__(self, **kwargs):
		self._content_fit_model = Gtk.StringList.new([value.value_nick for key, value in Gtk.ContentFit.__enum_values__.items()])
		self._paintable = None
		super().__init__(**kwargs)

		self.notify('content-fit-model')
		self.content_fit = Gtk.ContentFit.CONTAIN

	@Gtk.Template.Callback('on_open')
	def _on_open(self, button):
		dialog = Gtk.FileDialog(modal=True)
		filters = Gio.ListStore(item_type=Gtk.FileFilter)
		file_filter = Gtk.FileFilter(name='Image Files')
		file_filter.add_pixbuf_formats()
		filters.append(file_filter)
		dialog.set_filters(filters)
		dialog.open(self, None, self._on_select_image_file)

	def _on_select_image_file(self, source, result):
		try:
			file = source.open_finish(result)
			self._paintable = Gdk.Texture.new_from_file(file)
			self.notify('paintable')
		except GLib.Error as e:
			return

	@Gtk.Template.Callback('on_drop_image')
	def _on_drop_image(self, drop_target, value, x, y):
		if isinstance(value, Gio.File):
			self._paintable = Gdk.Texture.new_from_file(value)
			self.notify('paintable')
			return True
		elif isinstance(value, GdkPixbuf.Pixbuf):
			self._paintable = Gdk.Texture.new_for_pixbuf(value)
			self.notify('paintable')
			return True
		return False;

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

