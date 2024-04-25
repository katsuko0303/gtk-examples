# coding: utf-8

import sys, os, time
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, GLib, Pango, PangoCairo, Gdk

# instead of GLib.Timer
class Timer:
	def __init__(self):
		self._time = 0.0
		self._last_time = time.time()

	def is_active(self):
		return self._last_time is not None

	def continue_(self):
		self._last_time = time.time()

	def stop(self):
		if self._last_time is not None:
			self._time += time.time() - self._last_time
			self._last_time = None

	def destroy(self):
		pass

	def elapsed(self, n):
		if self._last_time is not None:
			now = time.time()
			self._time += now - self._last_time
			self._last_time = now
		return self._time

#----------------------------------------------------------------------
#	TimePaintable

class MyTimePaintable(GObject.GObject, Gdk.Paintable):
	__gtype_name__ = 'MyTimePaintable'

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=False)
	def is_running(self):
		return self._timer is not None and self._timer.is_active()

	@GObject.Signal(flags=GObject.SignalFlags.RUN_FIRST)
	def start(self):
		if self._timer is None:
			self._timer = Timer()
		else:
			if not self._timer.is_active():
				self._timer.continue_()
		self.invalidate_contents()

	@GObject.Signal(flags=GObject.SignalFlags.RUN_FIRST)
	def stop(self):
		if self._timer is not None and self._timer.is_active():
			self._timer.stop()
			self.invalidate_contents()

	@GObject.Signal(flags=GObject.SignalFlags.RUN_FIRST)
	def reset(self):
		if self._timer is not None:
			self._timer.destroy()
			self._timer = None
			self.invalidate_contents()

	def __init__(self, **kwargs):
		self._timer = None
		self._color = Gdk.RGBA()
		self._color.parse('black')
		super().__init__(**kwargs)

	def do_get_flags(self):
		return Gdk.PaintableFlags.SIZE

	def do_snapshot(self, snapshot, width, height):
		ctx = PangoCairo.FontMap.get_default().create_context()
		layout = Pango.Layout.new(ctx)

		font_desc = Pango.FontDescription.from_string('Sans')
		font_desc.set_absolute_size(height * Pango.SCALE)
		layout.set_font_description(font_desc)

		if self._timer is not None:
			milliseconds = int(self._timer.elapsed(None) * 1000)
		else:
			milliseconds = 0
		minutes = milliseconds // (60 * 1000)
		seconds = milliseconds // 1000 % 60
		ms = milliseconds // 10 % 100
		layout.set_text(f'{minutes:d}:{seconds:02d}.{ms:02d}')

		layout.set_width(width * Pango.SCALE)
		layout.set_height(height * Pango.SCALE)
		layout.set_alignment(Pango.Alignment.CENTER)

		snapshot.append_layout(layout, self._color)


#----------------------------------------------------------------------
#	Window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class MyWindow(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=GObject.TYPE_STRING)
	def play_icon_name(self):
		return 'media-playback-start-symbolic' if not self._paintable.is_running else 'media-playback-pause-symbolic'

	@GObject.Property(type=Gdk.Paintable)
	def paintable(self):
		return self._paintable

	def __init__(self, **kwargs):
		self._paintable = MyTimePaintable()
		self._tick_id = 0
		super().__init__(**kwargs)
		self.notify('paintable')

	@Gtk.Template.Callback('on_start_or_stop')
	def _on_start_or_stop(self, button):
		if self._paintable.is_running:
			self._paintable.stop()
			if self._tick_id != 0:
				self.remove_tick_callback(self._tick_id)
				self._tick_id = 0
		else:
			self._paintable.start()
			if self._tick_id == 0:
				self._tick_id = self.add_tick_callback(self._on_animate, None)
		self.notify('play-icon-name')

	@Gtk.Template.Callback('on_reset')
	def _on_reset(self, button):
		self._paintable.reset()
		if self._tick_id != 0:
			self.remove_tick_callback(self._tick_id)
			self._tick_id = 0

	def _on_animate(self, widget, clock, *args):
		self._paintable.invalidate_contents()
		return GLib.SOURCE_CONTINUE

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = MyWindow(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

