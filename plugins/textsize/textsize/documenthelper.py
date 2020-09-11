# -*- coding: utf-8 -*-
#
#  documenthelper.py - Document helper
#
#  Copyright (C) 2010 - Jesse van den Kieboom
#  Copyright (C) 2017 - Linux Mint team
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02110-1301, USA.


from .signals import Signals
from gi.repository import Gtk, Gdk, Pango

MAX_FONT_SIZE = 30
MIN_FONT_SIZE = 5

class DocumentHelper(Signals):
    def __init__(self, view):
        Signals.__init__(self)

        self._view = view

        self.connect_signal(self._view, 'scroll-event', self.on_scroll_event)
        self.connect_signal(self._view, 'button-press-event', self.on_button_press_event)

        self._view.textsize_document_helper = self

        self._default_font = None
        self._last_font = None

    def stop(self):
        if self._default_font:
            self._view.override_font(self._default_font)

        self.disconnect_signals(self._view)

        self._view.textsize_document_helper = None

    def update_default_font(self):
        context = self._view.get_style_context()
        description = context.get_font(context.get_state()).copy()

        if not self._last_font or description.hash() != self._last_font.hash():
            self._default_font = description

    def set_font_size(self, amount):
        self.update_default_font()

        context = self._view.get_style_context()
        description = context.get_font(context.get_state()).copy()

        buf = self._view.get_buffer()
        size = description.get_size() / Pango.SCALE

        if size >= MAX_FONT_SIZE and amount == 1:
            return;
        if size <= MIN_FONT_SIZE and amount == -1:
            return;

        description.set_size(max(1, (size + amount)) * Pango.SCALE)

        self._view.override_font(description)
        self._last_font = description

    def larger_text(self):
        self.set_font_size(1)

    def smaller_text(self):
        self.set_font_size(-1)

    def normal_size(self):
        self.update_default_font()

        buf = self._view.get_buffer()

        self._view.override_font(self._default_font)
        self._last_font = self._default_font

    def on_scroll_event(self, view, event):
        state = event.state & Gtk.accelerator_get_default_mod_mask()

        if state != Gdk.ModifierType.CONTROL_MASK:
            return False

        if event.direction == Gdk.ScrollDirection.UP:
            self.larger_text()
            return True
        elif event.direction == Gdk.ScrollDirection.DOWN:
            self.smaller_text()
            return True
        elif event.direction == Gdk.ScrollDirection.SMOOTH:
            if event.delta_y > 0:
                self.smaller_text()
            elif event.delta_y < 0:
                self.larger_text()

        return False

    def on_button_press_event(self, view, event):
        state = event.state & Gtk.accelerator_get_default_mod_mask()

        if state == Gdk.ModifierType.CONTROL_MASK and event.button == 2:
            self.normal_size()
            return True
        else:
            return False
