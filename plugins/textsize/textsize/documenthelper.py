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
        self._font_tags = {}

    def stop(self):
        if self._default_font:
            self._view.override_font(self._default_font)

        self.remove_font_tags()
        self.disconnect_signals(self._view)

        self._view.textsize_document_helper = None

    def remove_font_tags(self):
        buf = self._view.get_buffer()
        table = buf.get_tag_table()

        # Remove all the font tags
        for size in self._font_tags:
            tag = self._font_tags[size]
            table.remove(tag)

        self._font_tags = {}

    def update_default_font(self):
        context = self._view.get_style_context()
        description = context.get_font(context.get_state()).copy()

        if not self._last_font or description.hash() != self._last_font.hash():
            self._default_font = description

    def get_font_tags(self, start, end):
        tags = set()

        # Check all the know font tags
        for size in self._font_tags:
            tag = self._font_tags[size]

            if start.has_tag(tag):
                tags.add(tag)
            else:
                cp = start.copy()

                if cp.forward_to_tag_toggle(tag) and cp.compare(end) < 0:
                    tags.add(tag)

        return list(tags)

    def set_font_size(self, amount):
        self.update_default_font()

        context = self._view.get_style_context()
        description = context.get_font(context.get_state()).copy()

        buf = self._view.get_buffer()
        bounds = buf.get_selection_bounds()
        size = description.get_size() / Pango.SCALE

        if not bounds:
            if size >= MAX_FONT_SIZE and amount == 1:
                return;
            if size <= MIN_FONT_SIZE and amount == -1:
                return;

            description.set_size(max(1, (size + amount)) * Pango.SCALE)

            self._view.override_font(description)
            self._last_font = description
        else:
            start = bounds[0]
            end = bounds[1]

            tags = self.get_font_tags(start, end)

            if not tags:
                # Simply use the overall font size as the base
                newsize = size + amount
            elif len(tags) == 1:
                newsize = tags[0].props.font_desc.get_size() / Pango.SCALE + amount
            else:
                newsize = 0

                for tag in tags:
                    newsize += tag.props.font_desc.get_size() / Pango.SCALE

                newsize = round(newsize / len(tags))

            if newsize >= MAX_FONT_SIZE and amount == 1:
                return;
            if newsize <= MIN_FONT_SIZE and amount == -1:
                return;

            newsize = int(max(1, newsize))

            if not newsize in self._font_tags:
                newtag = buf.create_tag(None)

                desc = description
                desc.set_size(newsize * Pango.SCALE)

                newtag.props.font_desc = desc
                self._font_tags[newsize] = newtag
            else:
                newtag = self._font_tags[newsize]

            # Remove all the previous mix of tags
            for tag in tags:
                buf.remove_tag(tag, start, end)

            buf.apply_tag(newtag, start, end)

    def larger_text(self):
        self.set_font_size(1)

    def smaller_text(self):
        self.set_font_size(-1)

    def normal_size(self):
        self.update_default_font()

        buf = self._view.get_buffer()
        bounds = buf.get_selection_bounds()

        if not bounds:
            self.remove_font_tags()

            self._view.override_font(self._default_font)
            self._last_font = self._default_font
        else:
            tags = self.get_font_tags(bounds[0], bounds[1])

            for tag in tags:
                buf.remove_tag(tag, bounds[0], bounds[1])

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
