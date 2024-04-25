# coding: utf-8

import sys, os, traceback, math
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gdk, GLib, Graphene
import cairo

#----------------------------------------------------------------------
#	paintable

class Paintable(GObject.GObject, Gdk.Paintable):
	__gtype_name__ = 'MyPaintable'

	DEFAULT_SURFACE_WIDTH = 256
	DEFAULT_SURFACE_HEIGHT = 256

	@GObject.Property(type=GObject.TYPE_INT, minimum=1, maximum=GLib.MAXINT, default=DEFAULT_SURFACE_WIDTH)
	def surface_width(self):
		return self._surface_width if hasattr(self, '_surface_width') else self.DEFAULT_SURFACE_WIDTH
	@surface_width.setter
	def surface_width(self, value):
		self._surface_width = value

	@GObject.Property(type=GObject.TYPE_INT, minimum=1, maximum=GLib.MAXINT, default=DEFAULT_SURFACE_HEIGHT)
	def surface_height(self):
		return self._surface_height if hasattr(self, '_surface_height') else self.DEFAULT_SURFACE_HEIGHT
	@surface_height.setter
	def surface_height(self, value):
		self._surface_height = value

	@GObject.Signal(flags=GObject.SignalFlags.RUN_FIRST, arg_types=(GObject.TYPE_PYOBJECT,))
	def draw_overlapped(self, cr):
		pass

	def __init__(self, **kwargs):
		self._surface = None
		super().__init__(**kwargs)

		self._idle_id = GLib.idle_add(self._on_idle)

	def do_get_flags(self):
		return Gdk.PaintableFlags.SIZE

	def do_snapshot(self, snapshot, width, height):
		snapshot.save()

		bounds = Graphene.Rect().init(0, 0, width, height)

		color = Gdk.RGBA()
		color.red, color.green, color.blue, color.alpha = 0.2, 0.2, 0.2, 1.0
		snapshot.append_color(color, bounds)

		if self._surface is not None:
			cr = snapshot.append_cairo(bounds)
			cr.translate(width / 2.0, height / 2.0)
			cr.translate(-self.surface_width / 2.0, -self.surface_height / 2.0)

			cr.set_source_surface(self._surface, 0, 0)
			cr.paint()

			self.emit('draw-overlapped', cr)

		snapshot.restore()

	def _on_idle(self):
		self._surface = cairo.ImageSurface(cairo.FORMAT_RGB24, self.surface_width, self.surface_height)
		cr = cairo.Context(self._surface)
		cr.set_operator(cairo.OPERATOR_SOURCE)
		cr.set_source_rgb(1.0, 1.0, 1.0)
		cr.rectangle(0, 0, self.surface_width, self.surface_height)
		cr.fill()

		self.invalidate_contents()
		self._idle_id = 0
		return GLib.SOURCE_REMOVE

	def make_cairo_context(self):
		return cairo.Context(self._surface) if self._surface is not None else None

	def convert_to_surface_coordinate(self, x, y, width, height):
		sx = x - width / 2.0 + self.surface_width / 2.0
		sy = y - height / 2.0 + self.surface_height / 2.0
		return sx >= 0 and sx < self.surface_width and sy >= 0 and sy < self.surface_height, sx, sy

#----------------------------------------------------------------------
#	drawer

class Drawer(GObject.GObject):
	__gtype_name__ = 'MyDrawer'

	@GObject.Property(type=GObject.TYPE_STRING, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.CONSTRUCT_ONLY))
	def label(self):
		return self._label if hasattr(self, '_label') else ''
	@label.setter
	def label(self, value):
		self._label = value

	@GObject.Signal(flags=(GObject.SignalFlags.RUN_FIRST | GObject.SignalFlags.ACTION),
				 arg_types=(Paintable, Gtk.GestureDrag, GObject.TYPE_DOUBLE, GObject.TYPE_DOUBLE))
	def drag_begin(self, paintable, drag, start_x, start_y):
		pass

	@GObject.Signal(flags=(GObject.SignalFlags.RUN_FIRST | GObject.SignalFlags.ACTION),
				 arg_types=(Paintable, Gtk.GestureDrag, GObject.TYPE_DOUBLE, GObject.TYPE_DOUBLE))
	def drag_update(self, paintable, drag, offset_x, offset_y):
		pass

	@GObject.Signal(flags=(GObject.SignalFlags.RUN_FIRST | GObject.SignalFlags.ACTION),
				 arg_types=(Paintable, Gtk.GestureDrag, GObject.TYPE_DOUBLE, GObject.TYPE_DOUBLE))
	def drag_end(self, paintable, drag, offset_x, offset_y):
		pass

	@GObject.Signal(flags=GObject.SignalFlags.RUN_FIRST, return_type=Gtk.Widget)
	def make_form(self):
		raise NotImplementedError

	@GObject.Signal(flags=GObject.SignalFlags.RUN_FIRST, arg_types=(GObject.TYPE_PYOBJECT,))
	def draw_overlapped(self, cr):
		pass

	def __init__(self, **kwargs):
		super().__init__(**kwargs)

class LineDrawer(Drawer):
	__gtype_name__ = 'MyLineDrawer'

	@GObject.Property(type=Gdk.RGBA, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def color(self):
		if not hasattr(self, '_color'):
			color = Gdk.RGBA()
			color.parse('black')
		else:
			color = self._color
		return color
	@color.setter
	def color(self, value):
		if not hasattr(self, '_color') or not Gdk.RGBA.equal(value, self._color):
			self._color = value
			self.notify('color')

	@GObject.Property(type=GObject.TYPE_INT, minimum=1, maximum=GLib.MAXINT, default=2, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def line_width(self):
		return self._line_width if hasattr(self, '_line_width') else 2
	@line_width.setter
	def line_width(self, value):
		if not hasattr(self, '_line_width') or value != self._line_width:
			self._line_width = value
			self.notify('line-width')

	def __init__(self, **kwargs):
		self._dragging = None
		super().__init__(label='Line', **kwargs)

	def do_make_form(self):
		builder = Gtk.Builder()
		builder.add_from_file(os.path.join(os.path.dirname(__file__), 'line_drawer.ui'))

		color_button = builder.get_object('color_button')
		color_button.set_rgba(self.color)
		self.bind_property('color', color_button, 'rgba', GObject.BindingFlags.BIDIRECTIONAL)

		line_width_adjustment = builder.get_object('line_width_adjustment')
		line_width_adjustment.set_value(self.line_width)
		self.bind_property('line-width', line_width_adjustment, 'value', GObject.BindingFlags.BIDIRECTIONAL)

		return builder.get_object('root')

	def do_drag_begin(self, paintable, drag, start_x, start_y):
		widget = drag.get_widget()
		in_area, x, y = paintable.convert_to_surface_coordinate(start_x, start_y, widget.get_width(), widget.get_height())
		if in_area:
			self._dragging = x, y

	def _update_for_dragging(self, paintable, drag, offset_x, offset_y):
		if self._dragging is None:
			return
		last_x, last_y = self._dragging
		widget = drag.get_widget()
		active, sx, sy = drag.get_start_point()
		in_area, x, y = paintable.convert_to_surface_coordinate(sx + offset_x, sy + offset_y, widget.get_width(), widget.get_height())

		cr = paintable.make_cairo_context()
		if cr is not None:

			cr.set_operator(cairo.OPERATOR_SOURCE)
			Gdk.cairo_set_source_rgba(cr, self.color)

			cr.set_line_width(self.line_width)
			cr.move_to(last_x, last_y)
			cr.line_to(x, y)
			cr.stroke()

			cr.arc(last_x, last_y, self.line_width / 2.0, 0, 2.0 * math.pi)
			cr.arc(x, y, self.line_width / 2.0, 0, 2.0 * math.pi)
			cr.fill()

		self._dragging = x, y

	def do_drag_update(self, paintable, drag, offset_x, offset_y):
		self._update_for_dragging(paintable, drag, offset_x, offset_y)

	def do_drag_end(self, paintable, drag, offset_x, offset_y):
		self._update_for_dragging(paintable, drag, offset_x, offset_y)
		self._dragging = None

class RectAreaDrawer(Drawer):
	__gtype_name__ = 'MyRectAreaDrawer'

	@GObject.Signal(flags=GObject.SignalFlags.RUN_FIRST,
				 arg_types=(GObject.TYPE_PYOBJECT, GObject.TYPE_DOUBLE, GObject.TYPE_DOUBLE, GObject.TYPE_DOUBLE, GObject.TYPE_DOUBLE))
	def update_surface(self, cr, x, y, width, height):
		pass

	def __init__(self, **kwargs):
		self._dragging = None
		super().__init__(**kwargs)

	def do_drag_begin(self, paintable, drag, start_x, start_y):
		widget = drag.get_widget()
		in_area, x, y = paintable.convert_to_surface_coordinate(start_x, start_y, widget.get_width(), widget.get_height())
		if in_area:
			self._dragging = x, y, x, y

	def do_drag_update(self, paintable, drag, offset_x, offset_y):
		if self._dragging is not None:
			widget = drag.get_widget()
			x0, y0, x1, y1 = self._dragging
			active, sx, sy = drag.get_start_point()
			in_area, x1, y1 = paintable.convert_to_surface_coordinate(sx + offset_x, sy + offset_y, widget.get_width(), widget.get_height())
			self._dragging = x0, y0, x1, y1

	def do_drag_end(self, paintable, drag, offset_x, offset_y):
		if self._dragging is not None:
			x0, y0, x1, y1 = self._dragging
			widget = drag.get_widget()
			active, sx, sy = drag.get_start_point()
			in_area, x1, y1 = paintable.convert_to_surface_coordinate(sx + offset_x, sy + offset_y, widget.get_width(), widget.get_height())

			x, width = (x0, x1 - x0) if x0 < x1 else (x1, x0 - x1)
			y, height = (y0, y1 - y0) if y0 < y1 else (y1, y0 - y1)
			if width > 0 and height > 0:
				cr = paintable.make_cairo_context()
				self.emit('update-surface', cr, x, y, width, height)

			self._dragging = None

	def do_draw_overlapped(self, cr):
		if self._dragging is None:
			return
		x0, y0, x1, y1 = self._dragging
		cr.save()

		x, width = (x0, x1 - x0) if x0 < x1 else (x1, x0 - x1)
		y, height = (y0, y1 - y0) if y0 < y1 else (y1, y0 - y1)
		if width > 0 and height > 0:
			cr.rectangle(x, y, width, height)

			cr.set_operator(cairo.OPERATOR_ADD)
			cr.set_source_rgba(1, 1, 1, 0.5)
			cr.fill_preserve()

			cr.set_operator(cairo.OPERATOR_SOURCE)
			cr.set_source_rgba(0, 0, 0, 1)
			cr.set_line_width(2)
			cr.stroke()


		cr.restore()

class RectDrawer(RectAreaDrawer):
	__gtype_name__ = 'MyRectDrawer'

	@GObject.Property(type=Gdk.RGBA, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def color(self):
		if not hasattr(self, '_color'):
			color = Gdk.RGBA()
			color.parse('black')
		else:
			color = self._color
		return color
	@color.setter
	def color(self, value):
		if not hasattr(self, '_color') or not Gdk.RGBA.equal(value, self._color):
			self._color = value
			self.notify('color')

	def __init__(self, **kwargs):
		super().__init__(label='Rectangle', **kwargs)

	def do_update_surface(self, cr, x, y, width, height):
		cr.rectangle(x, y, width, height)
		cr.set_operator(cairo.OPERATOR_SOURCE)
		Gdk.cairo_set_source_rgba(cr, self.color)
		cr.fill()

	def do_make_form(self):
		builder = Gtk.Builder()
		builder.add_from_file(os.path.join(os.path.dirname(__file__), 'rect_drawer.ui'))

		color_button = builder.get_object('color_button')
		color_button.set_rgba(self.color)
		self.bind_property('color', color_button, 'rgba', GObject.BindingFlags.BIDIRECTIONAL)

		return builder.get_object('root')

class EraseDrawer(RectAreaDrawer):
	__gtype_name = 'MyRectAreaDrawer'

	def __init__(self, **kwargs):
		super().__init__(label='Erase', **kwargs)

	def do_update_surface(self, cr, x, y, width, height):
		cr.set_operator(cairo.OPERATOR_CLEAR)
		cr.rectangle(x, y, width, height)
		cr.fill()

	def do_make_form(self):
		return Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=Gdk.Paintable)
	def paintable(self):
		return self._paintable

	_drawer_model = Gtk.Template.Child('drawer_model')
	_drawer_box = Gtk.Template.Child('drawer_box')
	_picture = Gtk.Template.Child('picture')

	def __init__(self, **kwargs):
		self._paintable = Paintable()
		self._paintable.connect('draw-overlapped', self._on_paintable_draw_overlapped)
		self._drawer_form = None
		self._drawer = None
		super().__init__(**kwargs)
		self.notify('paintable')
		for drawer_type in [LineDrawer, RectDrawer, EraseDrawer]:
			drawer = drawer_type()
			self._drawer_model.append(drawer)

	@Gtk.Template.Callback('on_drawer_drop_down_notify_selected')
	def _on_drawer_drop_down_notify_selection(self, obj, pspec):
		if self._drawer_form is not None:
			self._drawer_box.remove(self._drawer_form)
			self._drawer_form = None
		self._drawer = obj.get_model().get_item(obj.get_selected())
		self._drawer_form = self._drawer.emit('make-form')
		self._drawer_box.append(self._drawer_form)

	@Gtk.Template.Callback('on_picture_drag_begin')
	def _on_picture_drag_begin(self, drag, start_x, start_y):
		if self._drawer is not None:
			self._drawer.emit('drag-begin', self._paintable, drag, start_x, start_y)
			self._paintable.invalidate_contents()

	@Gtk.Template.Callback('on_picture_drag_update')
	def _on_picture_drag_update(self, drag, offset_x, offset_y):
		if self._drawer is not None:
			self._drawer.emit('drag-update', self._paintable, drag, offset_x, offset_y)
			self._paintable.invalidate_contents()

	@Gtk.Template.Callback('on_picture_drag_end')
	def _on_picture_drag_end(self, drag, offset_x, offset_y):
		if self._drawer is not None:
			self._drawer.emit('drag-end', self._paintable, drag, offset_x, offset_y)
			self._paintable.invalidate_contents()

	def _on_paintable_draw_overlapped(self, paintable, cr):
		if self._drawer is not None:
			self._drawer.emit('draw-overlapped', cr)

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

