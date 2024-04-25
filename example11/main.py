# coding: utf-8

import sys, os, traceback
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gdk, GLib, Gio

#----------------------------------------------------------------------
#	name_dialog

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'name_dialog.ui'))
class NameDialog(Gtk.Window):
	__gtype_name__ = 'MyNameDialog'

	@GObject.Property(type=GObject.TYPE_STRING, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def name(self):
		return self._name if hasattr(self, '_name') else ''
	@name.setter
	def name(self, value):
		if not hasattr(self, '_name') or value != self._name:
			self._name = value
			self.notify('name')
			self.notify('can-accept')

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=False)
	def can_accept(self):
		return len(self.name) > 0

	@GObject.Signal(flags=(GObject.SignalFlags.RUN_FIRST | GObject.SignalFlags.ACTION))
	def cancel(self):
		if self._task is not None:
			self._task.return_error(GLib.Error.new_literal(Gtk.DialogError.quark(), 'Dismissed by user', Gtk.DialogError.DISMISSED))
			self._task = None
		self.destroy()

	def __init__(self, **kwargs):
		self._task = None
		super().__init__(**kwargs)

	def do_close_request(self):
		self.emit('cancel')
		return False

	def ask(self, parent, cancellable, callback, *args):
		self.set_transient_for(parent)
		self._task = Gio.Task.new(self, cancellable, callback, *args)
		self.set_visible(True)

	def ask_finish(self, result):
		ret, obj = result.propagate_value()
		return obj if ret else None

	@Gtk.Template.Callback('on_cancel')
	def _on_cancel(self, button):
		self.emit('cancel')

	@Gtk.Template.Callback('on_ok')
	def _on_ok(self, button):
		if self._task is not None:
			value = GObject.Value(value_type=GObject.TYPE_STRING)
			value.set_string(self.name)
			self._task.return_value(value)
			self._task = None
		self.destroy()

NameDialog.add_shortcut(Gtk.Shortcut(trigger=Gtk.KeyvalTrigger(keyval=Gdk.KEY_Escape, modifiers=0), action=Gtk.SignalAction(signal_name='cancel')))

#----------------------------------------------------------------------
#	window

class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	def __init__(self, **kwargs):
		super().__init__(default_width=400, default_height=300, **kwargs)

		button = Gtk.Button(label='Push')
		button.connect('clicked', self._on_clicked)
		self.set_child(button)

	def _on_clicked(self, button):
		dialog = NameDialog(modal=True)
		dialog.ask(self, None, self._on_ask_name, button)

	def _on_ask_name(self, source, result, button):
		try:
			name = source.ask_finish(result)
			button.set_label(f'My name is {name}.')
		except GLib.Error as e:
			return

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

