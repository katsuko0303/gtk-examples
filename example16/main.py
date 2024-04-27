# coding: utf-8

import sys, os
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Pango, Gdk, Gio

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	_keyword = 'world'

	@GObject.Property(type=GObject.TYPE_STRING)
	def label(self):
		return 'Hello, world!'

	@GObject.Property(type=Pango.AttrList)
	def attributes(self):
		attr_list = Pango.AttrList()
		attr_list.insert(Pango.attr_foreground_new(65535, 0, 0))
		attr_list.insert(Pango.attr_size_new(Pango.SCALE * 24))
		attr_list.insert(Pango.attr_family_new('Serif'))
		for start, end in self._get_word_positions():
			attr = Pango.attr_weight_new(Pango.Weight.BOLD)
			attr.start_index = start
			attr.end_index = end
			attr_list.insert(attr)
		return attr_list

	@GObject.Property(type=Pango.AttrList)
	def attributes_by_string(self):
		s = Pango.version_check(1, 50, 0)
		if s:
			print(s, file=sys.stderr)
			return None
		attrs = ['0 -1 foreground #0000ff', f'0 -1 size {Pango.SCALE * 24}', '0 -1 family Serif']
		for start, end in self._get_word_positions():
			attrs += [f'{start} {end} weight bold']
		return Pango.AttrList.from_string(','.join(attrs))

	@GObject.Property(type=GObject.TYPE_STRING)
	def markup_label(self):
		markup_label = '<span foreground="#c0c000" size="24pt" font_family="Serif">'
		start = 0
		while True:
			pos = self.label.find(self._keyword, start)
			if pos == -1:
				markup_label += self.label[start:]
				break
			markup_label += self.label[start:pos] + '<span font_weight="bold">' + self._keyword + '</span>'
			start = pos + len(self._keyword)
		markup_label += '</span>'
		return markup_label

	def __init__(self, **kwargs):
		super().__init__(**kwargs)

		self.notify('label')
		self.notify('markup_label')
		self.notify('attributes')
		self.notify('attributes_by_string')

	def _get_word_positions(self):
		word = self._keyword.encode('utf-8')
		start = 0
		positions = []
		label = self.label.encode('utf-8')
		while True:
			pos = label.find(word, start)
			if pos == -1:
				break
			positions += [(pos, pos + len(word))]
			start = pos + len(word)
		return positions

#----------------------------------------------------------------------
#	main

def on_startup(app):
	provider = Gtk.CssProvider()
	provider.load_from_file(Gio.File.new_for_path(os.path.join(os.path.dirname(__file__), 'app.css')))
	Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(), provider, Gtk.STYLE_PROVIDER_PRIORITY_FALLBACK)

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('startup', on_startup)
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

