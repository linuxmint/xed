# -*- coding: utf-8 -*-
#
#  documenthelper.py - Multi Edit
#
#  Copyright (C) 2009 - Jesse van den Kieboom
#  Copyright (C) 2020 - Linux Mint team
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

#from gi.repository import GLib, GObject, Pango, PangoCairo, Gdk, Gtk
import xml.sax.saxutils
import gi
gi.require_version('PangoCairo', '1.20')
from gi.repository import GLib, GObject, Pango, PangoCairo, Gdk, Gtk, Xed
from .signals import Signals

class DocumentHelper(Signals):
    def __init__(self, view):
        Signals.__init__(self)

        #view.multiedit_document_helper = self

        self._view = view

        self._view.multiedit_document_helper = self
        self._buffer = None
        self._in_mode = False
        self._column_mode = None
        self._move_cursor = None
        self._previous_move_cursor = None

        self._edit_points = []
        self._multi_edited = False
        self._status = None
        self._status_timeout = 0
        self._delete_mode_id = 0

        self.connect_signal(self._view, 'notify::buffer', self.on_notify_buffer)
        self.connect_signal(self._view, 'key-press-event', self.on_key_press_event)
        self.connect_signal(self._view, 'draw', self.on_view_draw)
        self.connect_signal(self._view, 'style-set', self.on_view_style_set)
        self.connect_signal(self._view, 'undo', self.on_view_undo)
        self.connect_signal(self._view, 'copy-clipboard', self.on_copy_clipboard)
        self.connect_signal(self._view, 'cut-clipboard', self.on_cut_clipboard)
        self.connect_signal(self._view, 'paste-clipboard', self.on_paste_clipboard)
        self.connect_signal(self._view, 'query-tooltip', self.on_query_tooltip)

        self.connect_signal(self._view, 'move-cursor', self.on_move_cursor)
        self.connect_signal_after(self._view, 'move-cursor', self.on_move_cursor_after)

        try:
            self.connect_signal(self._view, 'smart-home-end', self.on_smart_home_end)
        except:
            pass

        self._view.props.has_tooltip = True

        self.reset_buffer(self._view.get_buffer())

        self.initialize_event_handlers()
        self.toggle_callback = None

    def get_view(self):
        return self._view

    def set_toggle_callback(self, callback, data):
        self.toggle_callback = lambda: callback(data)

    def enabled(self):
        return self._in_mode

    def _update_selection_tag(self):
        context = self._view.get_style_context()
        state = context.get_state()

        fg_rgba = context.get_color(state)
        bg_rgba = context.get_background_color(state)

        # TODO: Use GdkRGBA directly when possible
        fg = Gdk.Color(fg_rgba.red * 65535, fg_rgba.green * 65535, fg_rgba.blue * 65535)
        bg = Gdk.Color(bg_rgba.red * 65535, bg_rgba.green * 65535, bg_rgba.blue * 65535)

        self._selection_tag.props.foreground_gdk = fg
        self._selection_tag.props.background_gdk = bg

    def reset_buffer(self, newbuf):
        if self._buffer:
            self.disable_multi_edit()

            self.disconnect_signals(self._buffer)
            self._buffer.get_tag_table().remove(self._selection_tag)
            self._buffer.delete_mark(self._last_insert)

        self._buffer = None

        #if newbuf == None or not isinstance(newbuf, Gedit.Document):
        if newbuf == None or not isinstance(newbuf, Xed.Document):
            return

        if newbuf != None:
            self.connect_signal(newbuf, 'insert-text', self.on_insert_text_before)
            self.connect_signal_after(newbuf, 'insert-text', self.on_insert_text)

            self.connect_signal(newbuf, 'delete-range', self.on_delete_range_before)
            self.connect_signal_after(newbuf, 'delete-range', self.on_delete_range)

            self.connect_signal_after(newbuf, 'mark-set', self.on_mark_set)
            self.connect_signal(newbuf, 'notify::style-scheme', self.on_notify_style_scheme)

            self._selection_tag = newbuf.create_tag(None)
            self._selection_tag.set_priority(newbuf.get_tag_table().get_size() - 1)
            self._last_insert = newbuf.create_mark(None, newbuf.get_iter_at_mark(newbuf.get_insert()), True)

            self._update_selection_tag()

        self._buffer = newbuf

    def stop(self):
        self._cancel_column_mode()
        self.reset_buffer(None)

        self._view.multiedit_document_helper = None

        self.disconnect_signals(self._view)
        self._view = None

        if self._status_timeout != 0:
            GObject.source_remove(self._status_timeout)
            self._status_timeout = 0

        if self._delete_mode_id != 0:
            GObject.source_remove(self._delete_mode_id)
            self._delete_mode_id = 0

    def initialize_event_handlers(self):
        self._event_handlers = [
            [('Escape',), 0, self.do_escape_mode, True],
            [('Return',), 0, self.do_column_edit, True],
            [('Return',), Gdk.ModifierType.CONTROL_MASK, self.do_smart_column_edit, True],
            [('Return',), Gdk.ModifierType.CONTROL_MASK | Gdk.ModifierType.SHIFT_MASK, self.do_smart_column_align, True],
            [('Return',), Gdk.ModifierType.CONTROL_MASK | Gdk.ModifierType.SHIFT_MASK | Gdk.ModifierType.MOD1_MASK, self.do_smart_column_align, True],
            [('Home',), Gdk.ModifierType.CONTROL_MASK, self.do_mark_start, True],
            [('End',), Gdk.ModifierType.CONTROL_MASK, self.do_mark_end, True],
            [('e', 'E'), Gdk.ModifierType.CONTROL_MASK, self.do_toggle_edit_point, True]
        ]

        for handler in self._event_handlers:
            handler[0] = list(map(lambda x: Gdk.keyval_from_name(x), handler[0]))

    def disable_multi_edit(self):
        if self._column_mode:
            self._cancel_column_mode()

        self._in_mode = False

        self._view.set_border_window_size(Gtk.TextWindowType.TOP, 0)
        self.remove_edit_points()

        if self.toggle_callback:
            self.toggle_callback()

    def enable_multi_edit(self):
        self._view.set_border_window_size(Gtk.TextWindowType.TOP, 20)
        self._in_mode = True

        if self.toggle_callback:
            self.toggle_callback()

    def toggle_multi_edit(self, enabled):
        if self.enabled() == enabled:
            return

        if self.enabled():
            self.disable_multi_edit()
        else:
            self.enable_multi_edit()

    def remove_edit_points(self):
        buf = self._buffer

        for mark in self._edit_points:
            buf.delete_mark(mark)

        self._edit_points = []
        self._multi_edited = False
        self._view.queue_draw()

    def do_escape_mode(self, event):
        if self._column_mode:
            self._cancel_column_mode()
            return True

        if self._edit_points:
            self.remove_edit_points()
            return True

        self.disable_multi_edit()
        return True

    def iter_to_offset(self, piter):
        return self._view.get_visual_column(piter)

    def get_visible_iter(self, line, offset):
        piter = self._buffer.get_iter_at_line(line)
        tw = self._view.get_tab_width()
        visiblepos = 0

        while visiblepos < offset:
            if piter.get_char() == "\t":
                visiblepos += (tw - (visiblepos % tw))
            else:
                visiblepos += 1

            if not piter.forward_char() or piter.get_line() != line:
                if piter.get_line() != line:
                    piter.backward_char()
                    visiblepos -= 1

                return piter, offset - visiblepos

        return piter, offset - visiblepos

    def _delete_columns(self):
        # Delete the text currently selected in column mode
        # If a line ends before the column selection, simply don't do anything.
        # Convert any tabs in the column selection to spaces
        # Remove all characters on each line within the column selection
        mode = self._column_mode
        self._column_mode = None

        buf = self._buffer
        start = mode[0]
        end = mode[1]

        buf.begin_user_action()

        while start <= end:
            start_iter, start_offset = self.get_visible_iter(start, mode[2])
            start += 1

            if start_offset > 0:
                # Only insert spaces
                buf.insert(start_iter, ' ' * start_offset)
                continue

            prefix = ''

            if start_offset < 0:
                # We went one tab over the start, go one back
                start_iter.backward_char()
                prefix = ' ' * (-start_offset)

            # Get the end one
            end_iter, end_offset = self.get_visible_iter(start - 1, mode[3])
            suffix = ''

            if end_offset > 0:
                # Delete until end of line
                end_iter = start_iter.copy()

                if not end_iter.ends_line():
                    end_iter.forward_to_line_end()
            elif end_offset < 0:
                # Within tab
                suffix = ' ' * (-end_offset)

            buf.delete(start_iter, end_iter)
            buf.insert(start_iter, prefix + suffix)

        buf.end_user_action()

    def _add_edit_point(self, piter):
        # Check if there is already an edit point here
        marks = piter.get_marks()

        for mark in marks:
            if mark in self._edit_points:
                return

        buf = self._buffer
        mark = buf.create_mark(None, piter, True)
        mark.set_visible(True)

        self._edit_points.append(mark)
        self.status('<i>%s</i>' % (xml.sax.saxutils.escape(_('Added edit point...'),)))

    def _remove_duplicate_edit_points(self):
        buf = self._buffer

        for mark in list(self._edit_points):
            if mark.get_deleted():
                continue

            piter = buf.get_iter_at_mark(mark)

            others = piter.get_marks()
            others.remove(mark)

            for other in others:
                if other in self._edit_points:
                    buf.delete_mark(other)
                    self._edit_points.remove(other)

        marks = buf.get_iter_at_mark(buf.get_insert()).get_marks()

        for mark in marks:
            if mark in self._edit_points:
                buf.delete_mark(mark)
                self._edit_points.remove(mark)

    def _invalidate_status(self):
        if not self._in_mode:
            return

        window = self._view.get_window(Gtk.TextWindowType.TOP)
        geom = window.get_geometry()
        #FIXME: there should be some override in pygobject to create the rectangle with values
        rect = Gdk.Rectangle()
        rect.x = 0
        rect.y = 0
        rect.width = geom[2]
        rect.height = geom[3]
        window.invalidate_rect(rect, False)

    def _remove_status(self):
        self._status = None
        self._invalidate_status()

        self._status_timeout = 0
        return False

    def status(self, text):
        if not self._in_mode:
            self._status = None
            return

        self._status = text
        self._invalidate_status()

        if self._status_timeout != 0:
            GLib.source_remove(self._status_timeout)

        self._status_timeout = GLib.timeout_add(3000, self._remove_status)

    def _apply_column_mode(self):
        mode = self._column_mode

        # Delete the columns
        self._delete_columns()

        # Insert insertion marks at the start column
        start = mode[0]
        end = mode[1]
        column = mode[2]
        buf = self._buffer

        while start <= end:
            piter, offset = self.get_visible_iter(start, column)

            if offset != 0:
                sys.stderr.write('Wrong offset in applying column mode, should never happen: %d, %d' % (start, offset))
            elif start != end:
                # Add edit point for all lines, except last one
                self._add_edit_point(piter)
            else:
                # For last line, just move the insertion point there
                buf.move_mark(buf.get_insert(), piter)
                buf.move_mark(buf.get_selection_bound(), piter)

            start += 1

        self._view.queue_draw()

    def line_column_edit(self, piter, soff, eoff):
        start, soff = self.get_visible_iter(piter.get_line(), soff)
        end, eof = self.get_visible_iter(piter.get_line(), eoff)

        if eof < 0:
            end.backward_char()

        # Apply tag to start -> end
        if start.compare(end) < 0:
            self._buffer.apply_tag(self._selection_tag, start, end)

    def smart_column_iters(self):
        buf = self._buffer

        bounds = buf.get_selection_bounds()

        if not bounds or bounds[0].get_line() == bounds[1].get_line():
            return False

        # There are a few 'smart' things
        # 1) if there is a number at the cursor, match any number
        # 2) if there is a non-alpha/non-space at the cursor, match it
        # 3) match word position
        mm = buf.get_iter_at_mark(buf.get_selection_bound())

        ch = mm.get_char()
        insline = mm.get_line()

        regex = None

        if ch.isnumeric():
            regex = re.compile('[0-9]')
        elif not ch.isalpha() and not ch.isspace():
            regex = re.compile(re.escape(ch))
        elif not mm.starts_word():
            return True

        start = bounds[0].copy()
        end = bounds[1].copy()

        offset = self.iter_to_offset(mm)

        if not regex:
            # Get word number
            worditer = mm.copy()

            if not worditer.starts_line():
                worditer.set_line_offset(0)

            wordcount = 0

            while worditer.compare(start) < 0:
                if not worditer.forward_visible_word_end():
                    break

                wordcount += 1

        if not start.starts_line():
            start.set_line_offset(0)

        iters = []

        while start.compare(end) < 0:
            if start.get_line() != insline:
                lineend = start.copy()

                if not lineend.ends_line():
                    lineend.forward_to_line_end()

                if regex:
                    matches = regex.finditer(start.get_text(lineend))
                    closest = None
                    closest_diff = 0

                    for match in matches:
                        piter = start.copy()
                        piter.forward_chars(match.start(0))

                        diff = abs(self.iter_to_offset(piter) - offset)

                        if not closest or diff < closest_diff:
                            closest = piter
                            closest_diff = diff

                    if not matches:
                        return True
                else:
                    lineno = start.get_line()
                    cp = start.copy()
                    cnt = 0

                    if wordcount > 0 and cp.forward_visible_word_ends(wordcount):
                        if cp.backward_visible_word_start() and cp.get_line() == lineno:
                            closest = cp
                        else:
                            closest = start.copy()

                            if not closest.ends_line():
                                closest.forward_to_line_end()

                iters.append(closest)
            else:
                iters.append(mm)

            if not start.forward_line():
                break

        return iters

    def do_smart_column_align(self, event):
        if not self._edit_points:
            ret = self.smart_column_iters()

            if ret == True or ret == False:
                return ret
        else:
            iters = [self._buffer.get_iter_at_mark(x) for x in self._edit_points]
            iters.append(self._buffer.get_iter_at_mark(self._buffer.get_insert()))

            iters.sort(lambda a, b: a.compare(b))
            ret = []
            lastline = -1

            for piter in iters:
                line = piter.get_line()

                if line != lastline:
                    ret.append(piter)

                lastline = line

        lastline = self._buffer.get_iter_at_mark(self._buffer.get_insert()).get_line()

        # Get max visual offset
        offsets = []
        maxoffset = 0

        for piter in ret:
            offset = self.iter_to_offset(piter)

            cp = piter.copy()
            moffset = offset

            if (event.state & Gdk.ModifierType.MOD1_MASK) and cp.backward_visible_cursor_position() and not cp.get_char().isspace():
                moffset += 1

            if moffset > maxoffset:
                maxoffset = moffset

            offsets.append(offset)

        marks = [self._buffer.create_mark(None, x, False) for x in ret]

        # Remove previous edit points
        self.remove_edit_points()

        self.block_signal(self._buffer, 'mark-set')
        self.block_signal(self._buffer, 'insert-text')

        self._buffer.begin_user_action()

        for i in range(len(marks)):
            # Align with spaces on the left such that 'mark' is at maxoffset
            num = maxoffset - offsets[i]
            text = ' ' * num

            piter = self._buffer.get_iter_at_mark(marks[i])
            self._buffer.insert(piter, text)

            if piter.get_line() != lastline:
                self._add_edit_point(piter)
            else:
                self._buffer.place_cursor(piter)

        for mark in marks:
            self._buffer.delete_mark(mark)

        self._buffer.end_user_action()

        self.unblock_signal(self._buffer, 'mark-set')
        self.unblock_signal(self._buffer, 'insert-text')

        return True

    def do_smart_column_edit(self, event):
        ret = self.smart_column_iters()

        if ret == True or ret == False:
            return ret

        lastline = self._buffer.get_iter_at_mark(self._buffer.get_insert()).get_line()

        # Remove previous edit points
        self.remove_edit_points()

        for i in range(len(ret)):
            piter = ret[i]

            if piter.get_line() != lastline:
                self._add_edit_point(piter)
            else:
                self._buffer.place_cursor(piter)

        return True

    def do_column_edit(self, event):
        buf = self._buffer

        bounds = buf.get_selection_bounds()

        if not bounds or bounds[0].get_line() == bounds[1].get_line():
            return False

        # Determine the column edit range in character positions, for each line
        # in the selection, determine where to put the edit with respect to tabs
        # that might be in the selection. Set selection tags on normal text.
        # If the column starts or stops in a tab, do custom overlay drawing
        bounds[0].order(bounds[1])
        start = bounds[0]
        end = bounds[1]

        tw = self._view.get_tab_width()
        soff = self.iter_to_offset(start)
        eoff = self.iter_to_offset(end)

        if eoff < soff:
            tmp = soff
            soff = eoff
            eoff = tmp

        # Apply tags where possible
        start_line = start.get_line()
        end_line = end.get_line()

        singlecolumn = soff == eoff

        while start.get_line() <= end.get_line():
            self.line_column_edit(start, soff, eoff)

            singlecolumn = (singlecolumn and self.get_visible_iter(start.get_line(), soff)[1] == 0)

            if not start.forward_line():
                break

        # Remove official selection
        insert = buf.get_iter_at_mark(buf.get_insert())
        buf.move_mark(buf.get_selection_bound(), insert)

        # Remove previous marks
        self.remove_edit_points()

        # Set the column mode
        self._column_mode = (start_line, end_line, soff, eoff)
        #self.status('<i>%s</i>' % (xml.sax.saxutils.escape(_('Column Mode...')),))

        if singlecolumn:
            self._apply_column_mode()
            self._multi_edited = True

        return True

    def _draw_column_mode(self, cr):
        if not self._column_mode:
            return False

        start = self._column_mode[0]
        end = self._column_mode[1]
        buf = self._buffer

        layout = self._view.create_pango_layout('W')
        width = layout.get_pixel_extents()[1].width

        context = self._view.get_style_context()
        col = context.get_background_color(Gtk.StateFlags.SELECTED)
        Gdk.cairo_set_source_rgba(cr, col)

        cstart = self._column_mode[2]
        cend = self._column_mode[3]

        while start <= end:
            # Get the line range, convert to window coords, and see if it needs
            # rendering
            piter = buf.get_iter_at_line(start)
            y, height = self._view.get_line_yrange(piter)

            x_, y = self._view.buffer_to_window_coords(Gtk.TextWindowType.TEXT, 0, y)
            start += 1

            # Check where to possible draw fake selection
            start_iter, soff = self.get_visible_iter(start - 1, cstart)
            end_iter, eoff = self.get_visible_iter(start - 1, cend)

            if soff == 0 and eoff == 0 and not start_iter.equal(end_iter):
                continue

            rx = cstart * width + self._view.get_left_margin()
            rw = (cend - cstart) * width

            if rw == 0:
                rw = 1

            cr.rectangle(rx, y, rw, height)
            cr.fill()

        return False

    def do_mark_start_end(self, test, move):
        buf = self._buffer
        bounds = buf.get_selection_bounds()

        if bounds:
            start = bounds[0]
            end = bounds[1]
        else:
            start = buf.get_iter_at_mark(buf.get_insert())
            end = start.copy()

        start.order(end)
        orig = start.copy()

        while start.get_line() <= end.get_line():
            if not test or not test(start):
                move(start)

            self._add_edit_point(start)

            if not start.forward_line():
                break

        return orig, end

    def do_mark_start(self, event):
        start, end = self.do_mark_start_end(None, lambda x: x.set_line_offset(0))

        start.backward_line()

        buf = self._buffer
        buf.move_mark(buf.get_insert(), start)
        buf.move_mark(buf.get_selection_bound(), start)

        return True

    def do_mark_end(self, event):
        start, end = self.do_mark_start_end(lambda x: x.ends_line(), lambda x: x.forward_to_line_end())

        end.forward_line()

        if not end.ends_line():
            end.forward_to_line_end()

        buf = self._buffer
        buf.move_mark(buf.get_insert(), end)
        buf.move_mark(buf.get_selection_bound(), end)

        return True

    def do_toggle_edit_point(self, event):
        buf = self._buffer
        piter = buf.get_iter_at_mark(buf.get_insert())

        marks = piter.get_marks()

        for mark in marks:
            if mark in self._edit_points:
                buf.delete_mark(mark)
                self._edit_points.remove(mark)

                #self.status('<i>%s</i>' % (xml.sax.saxutils.escape(_('Removed edit point...'),)))
                return

        self._add_edit_point(piter)
        return True

    def on_key_press_event(self, view, event):
        defmod = Gtk.accelerator_get_default_mod_mask() & event.state

        for handler in self._event_handlers:
            if (not handler[3] or self._in_mode) and event.keyval in handler[0] and (defmod == handler[1]):
                return handler[2](event)

        return False

    def on_notify_style_scheme(self, buf, spec):
        self._update_selection_tag()

    def on_view_style_set(self, view, prev):
        self._update_selection_tag()

    def on_notify_buffer(self, view, spec):
        self.reset_buffer(view.get_buffer())

    def on_insert_text_before(self, buf, where, text, length):
        if not self._in_mode:
            return

        self._remove_duplicate_edit_points()

    def on_insert_text(self, buf, where, text, length):
        if not self._in_mode:
            return

        self.block_signal(buf, 'insert-text')
        buf.begin_user_action()

        insert = buf.get_iter_at_mark(buf.get_insert())
        atinsert = where.equal(insert)
        wasat = buf.create_mark(None, where, True)

        if self._column_mode:
            self._apply_column_mode()

        if self._edit_points and atinsert:
            # Insert the text at all the edit points
            for mark in self._edit_points:
                piter = buf.get_iter_at_mark(mark)

                if not buf.get_iter_at_mark(buf.get_insert()).equal(piter):
                    self._multi_edited = True
                    buf.insert(piter, text)
        else:
            self.remove_edit_points()

        iterwas = buf.get_iter_at_mark(wasat)
        where.assign(iterwas)

        if atinsert:
            buf.move_mark(buf.get_insert(), iterwas)
            buf.move_mark(buf.get_selection_bound(), iterwas)

        buf.delete_mark(wasat)
        buf.end_user_action()
        self.unblock_signal(buf, 'insert-text')

    def on_delete_range_before(self, buf, start, end):
        if not self._in_mode:
            return

        self._remove_duplicate_edit_points()
        self._delete_text = start.get_text(end)
        self._delete_length = abs(end.get_offset() - start.get_offset())

        start.order(end)
        self._is_backspace = start.compare(buf.get_iter_at_mark(buf.get_insert())) < 0

    def handle_column_mode_delete(self, mark):
        buf = self._buffer
        start = buf.get_iter_at_mark(mark)

        self._view.set_editable(True)

        # Reinsert what was deleted, and apply column mode
        self.block_signal(buf, 'insert-text')

        singlecolumn = self._column_mode[2] == self._column_mode[3]

        buf.begin_user_action()
        buf.insert(start, self._delete_text)

        start = buf.get_iter_at_mark(mark)
        buf.delete_mark(mark)

        self._apply_column_mode()
        buf.end_user_action()

        self.unblock_signal(buf, 'insert-text')
        self._delete_mode_id = 0

        if singlecolumn:
            # Redo the delete actually
            end = start.copy()
            end.forward_char()

            buf.delete(start, end)

        return False

    def on_delete_range(self, buf, start, end):
        if self._column_mode:
            # Ooooh, what a hack to be able to work with the undo manager
            self._view.set_editable(False)
            mark = buf.create_mark(None, start, True)
            self._delete_mode_id = GObject.timeout_add(0, self.handle_column_mode_delete, mark)
        elif self._edit_points:
            if start.equal(buf.get_iter_at_mark(buf.get_insert())):
                self.block_signal(buf, 'delete-range')
                buf.begin_user_action()
                orig = buf.create_mark(None, start, True)

                for mark in self._edit_points:
                    piter = buf.get_iter_at_mark(mark)
                    other = piter.copy()

                    if self._is_backspace:
                        # Remove 'delete_length' chars _before_ piter
                        if not other.backward_chars(self._delete_length):
                            continue
                    else:
                        # Remove 'delete_text' chars _after_ piter
                        if not other.forward_chars(self._delete_length):
                            continue

                    if piter.equal(other):
                        continue

                    piter.order(other)
                    buf.delete(piter, other)
                    self._multi_edited = True

                buf.end_user_action()
                self.unblock_signal(buf, 'delete-range')

                piter = buf.get_iter_at_mark(orig)
                buf.delete_mark(orig)

                start.assign(piter)
                end.assign(piter)
            else:
                self.remove_edit_points()

    def _cancel_column_mode(self):
        if not self._column_mode:
            return

        self._column_mode = None

        buf = self._buffer
        bounds = buf.get_bounds()

        buf.remove_tag(self._selection_tag, bounds[0], bounds[1])

        #self.status('<i>%s</i>' % (xml.sax.saxutils.escape(_('Cancelled column mode...'),)))
        self._view.queue_draw()

    def _column_text(self):
        if not self._column_mode:
            return ''

        start = self._column_mode[0]
        end = self._column_mode[1]
        buf = self._buffer

        cstart = self._column_mode[2]
        cend = self._column_mode[3]

        lines = []
        width = cend - cstart

        while start <= end:
            start_iter, soff = self.get_visible_iter(start, cstart)
            end_iter, eoff = self.get_visible_iter(start, cend)

            if soff == 0 and eoff == 0:
                # Just text
                lines.append(start_iter.get_text(end_iter))
            elif (soff < 0 and eoff < 0) or soff > 0:
                # Only spaces
                lines.append(' ' * width)
            elif soff < 0:
                # start to end_iter
                lines.append((' ' * abs(soff)) + start_iter.get_text(end_iter))
            elif eoff != 0:
                # Draw from start_iter to end
                if eoff < 0:
                    end_iter.backward_char()
                    eoff = self._view.get_tab_width() + eoff

                lines.append(start_iter.get_text(end_iter) + (' ' * abs(eoff)))
            else:
                lines.append('')

            start += 1

        return "\n".join(lines)

    def on_copy_clipboard(self, view):
        if not self._column_mode:
            return

        text = self._column_text()

        clipboard = Gtk.Clipboard.get_for_display(self._view.get_display(), Gdk.SELECTION_CLIPBOARD)
        clipboard.set_text(text, -1)

        view.stop_emission('copy-clipboard')

    def on_cut_clipboard(self, view):
        if not self._column_mode:
            return

        text = self._column_text()

        clipboard = Gtk.Clipboard.get_for_display(self._view.get_display(), Gdk.SELECTION_CLIPBOARD)
        clipboard.set_text(text, -1)

        view.stop_emission('cut-clipboard')

        self._apply_column_mode()

    def on_clipboard_text(self, clipboard, text, data):
        # Check if the number of lines in the text matches the number of edit
        # points

        lines = []

        if text:
            lines = text.splitlines()

        buf = self._buffer
        piter = buf.get_iter_at_mark(self._paste_mark)
        ins = buf.get_iter_at_mark(buf.get_insert())

        if len(lines) != (len(self._edit_points) + 1) or piter.compare(ins) != 0:
            # Actually, the buffer better handle it...
            self.block_signal(self._view, 'paste-clipboard')
            buf.paste_clipboard(clipboard, piter, True)
            self.unblock_signal(self._view, 'paste-clipboard')
        else:
            # Insert text at each of the edit points then
            self.block_signal(buf, 'insert-text')
            self.block_signal(self._view, 'mark-set')

            buf.begin_user_action()

            marks = list(self._edit_points)
            marks.append(buf.get_insert())

            marks.sort(lambda a, b: buf.get_iter_at_mark(a).compare(buf.get_iter_at_mark(b)))

            for i in range(len(lines)):
                piter = buf.get_iter_at_mark(marks[i])
                buf.insert(piter, lines[i])

                if marks[i] != buf.get_insert():
                    buf.move_mark(marks[i], piter)

            buf.end_user_action()

            self.unblock_signal(buf, 'insert-text')
            self.unblock_signal(self._view, 'mark-set')

        buf.delete_mark(self._paste_mark)

    def on_paste_clipboard(self, view):
        if not self._edit_points:
            return

        clipboard = Gtk.Clipboard.get_for_display(self._view.get_display(), Gdk.SELECTION_CLIPBOARD)
        self._paste_mark = self._buffer.create_mark(None, self._buffer.get_iter_at_mark(self._buffer.get_insert()), True)

        clipboard.request_text(self.on_clipboard_text, None)
        view.stop_emission('paste-clipboard')

    def _move_edit_points(self, buf, where):
        diff = where.get_offset() - buf.get_iter_at_mark(self._last_insert).get_offset()

        for point in self._edit_points:
            piter = buf.get_iter_at_mark(point)
            piter.set_offset(piter.get_offset() + diff)
            buf.move_mark(point, piter)

    def _move_edit_point_logical_positions(self, piter, count):
        piter.forward_cursor_positions(count)

        return piter

    def _move_edit_point_visual_positions(self, piter, count):
        self._view.move_visually(piter, count)

        return piter

    def _move_edit_point_words(self, piter, count):
        if count > 0:
            piter.forward_visible_word_ends(count)
        else:
            piter.backward_visible_word_starts(-count)

        return piter

    def _move_edit_point_display_lines(self, piter, count):
        offset = self.iter_to_offset(piter)

        if count < 0:
            cnt = -count
        else:
            cnt = count

        for i in range(cnt):
            if count > 0:
                self._view.forward_display_line(piter)
            else:
                self._view.backward_display_line(piter)

        piter, off = self.get_visible_iter(piter.get_line(), offset)

        return piter

    def _move_to_first_char(self, piter, display_line):
        last = piter.copy()

        if display_line:
            self._view.forward_display_line_end(last)
            self._view.backward_display_line_start(piter)
        else:
            piter.set_line_offset(0)

            if not last.ends_line():
                last.forward_to_line_end()

        while piter.compare(last) < 0:
            if piter.get_char().isspace():
                piter.forward_visible_cursor_position()
            else:
                break

    def _move_to_last_char(self, piter, display_line):
        first = piter.copy()

        if display_line:
            self._view.backward_display_line_start(first)
            self._view.forward_display_line_end(piter)
        else:
            if not piter.ends_line():
                piter.forward_to_line_end()

            first.set_line_offset(0)

        while piter.compare(first) > 0:
            piter.backward_visible_cursor_position()

            if not piter.get_char().isspace():
                if not piter.forward_visible_cursor_position():
                    piter.forward_to_end()

                break

    def _move_edit_point_smart_display_line_ends(self, piter, count):
        if count > 0:
            self._move_to_last_char(piter, True)
        else:
            self._move_to_first_char(piter, True)

        return piter

    def _move_edit_point_display_line_ends(self, piter, count):
        if count > 0:
            self._view.forward_display_line_end(piter)
        else:
            self._view.backward_display_line_start(piter)

        return piter

    def _move_edit_point_paragraphs(self, piter, count):
        offset = self.iter_to_offset(piter)

        piter.forward_visible_lines(count)
        piter, off = self.get_visible_iter(piter.get_line(), offset)

        return piter

    def _move_edit_point_smart_paragraph_ends(self, piter, count):
        if count > 0:
            self._move_to_last_char(piter, False)
        else:
            self._move_to_first_char(piter, False)

        return piter

    def _move_edit_point_paragraph_ends(self, piter, count):
        if count > 0:
            if not piter.ends_line():
                piter.forward_to_line_end()
        elif not piter.starts_line():
            piter.set_line_offset(0)

        return piter

    def _move_edit_point_horizontal_pages(self, piter, count):
        return self._move_edit_point_paragraph_ends(piter, count)

    def _move_edit_points_by_cursor(self, buf, where):
        actions = {
            Gtk.MovementStep.LOGICAL_POSITIONS: self._move_edit_point_logical_positions,
            Gtk.MovementStep.VISUAL_POSITIONS: self._move_edit_point_visual_positions,
            Gtk.MovementStep.WORDS: self._move_edit_point_words,
            Gtk.MovementStep.DISPLAY_LINES: self._move_edit_point_display_lines,
            Gtk.MovementStep.DISPLAY_LINE_ENDS: self._move_edit_point_display_line_ends,
            Gtk.MovementStep.PARAGRAPHS: self._move_edit_point_paragraphs,
            Gtk.MovementStep.PARAGRAPH_ENDS: self._move_edit_point_paragraph_ends,
            Gtk.MovementStep.HORIZONTAL_PAGES: self._move_edit_point_horizontal_pages,
        }

        typ = self._move_cursor[0]
        count = self._move_cursor[1]

        if not typ in actions:
            return self._move_edit_points(buf, where)

        action = actions[typ]

        for point in self._edit_points:
            piter = buf.get_iter_at_mark(point)
            piter = action(piter, count)
            buf.move_mark(point, piter)

        self._previous_move_cursor = self._move_cursor
        self._move_cursor = None

    def on_mark_set(self, buf, where, mark):
        if not mark == buf.get_insert():
            return

        if self._in_mode:
            if self._column_mode != None:
                # Cancel column mode when cursor moves
                self._cancel_column_mode()
            elif self._edit_points and self._multi_edited:
                if self._move_cursor != None:
                    self._move_edit_points_by_cursor(buf, where)
                else:
                    self._move_edit_points(buf, where)

                self._remove_duplicate_edit_points()

        self._buffer.move_mark(self._last_insert, where)

    def on_view_undo(self, view):
        self._cancel_column_mode()
        self.remove_edit_points()

    def make_label(self, text, use_markup=True):
        lbl = Gtk.Label(text)
        lbl.set_use_markup(use_markup)
        lbl.set_alignment(0, 0.5)
        lbl.show()

        return lbl

    def on_query_tooltip(self, view, x, y, keyboard_mode, tooltip):
        if not self._in_mode:
            return False

        geom = view.get_window(Gtk.TextWindowType.TOP).get_geometry()

        if x < geom[0] or x > geom[0] + geom[2] or y < geom[1] or y > geom[1] + geom[3]:
            return False

        table = Gtk.Table(13, 2)
        table.set_row_spacings(3)
        table.set_col_spacings(12)

        table.attach(self.make_label('<b>Selection</b>', True),
                     0, 2, 0, 1,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)
        table.attach(self.make_label('<Enter>', False),
                     0, 1, 1, 2,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)
        table.attach(self.make_label('<Ctrl><Enter>', False),
                     0, 1, 2, 3,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)
        table.attach(self.make_label('<Ctrl><Shift><Enter>', False),
                     0, 1, 3, 4,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)
        table.attach(self.make_label('<Ctrl><Alt><Shift><Enter>', False),
                     0, 1, 4, 5,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)

        sep = Gtk.HSeparator()
        sep.show()

        table.attach(sep,
                     0, 2, 5, 6,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)
        table.attach(self.make_label('<b>Edit points</b>', True),
                     0, 2, 6, 7,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)
        table.attach(self.make_label('<Ctrl>+E', False),
                     0, 1, 7, 8,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)
        table.attach(self.make_label('<Ctrl><Home>', False),
                     0, 1, 8, 9,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)
        table.attach(self.make_label('<Ctrl><End>', False),
                     0, 1, 9, 10,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)
        table.attach(self.make_label('<Ctrl><Shift><Enter>', False),
                     0, 1, 10, 11,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)
        table.attach(self.make_label('<Ctrl><Alt><Shift><Enter>', False),
                     0, 1, 11, 12,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL,
                     Gtk.AttachOptions.SHRINK | Gtk.AttachOptions.FILL)

        table.attach(self.make_label(_('Enter column edit mode using selection')), 1, 2, 1, 2)
        table.attach(self.make_label(_('Enter <b>smart</b> column edit mode using selection')), 1, 2, 2, 3)
        table.attach(self.make_label(_('<b>Smart</b> column align mode using selection')), 1, 2, 3, 4)
        table.attach(self.make_label(_('<b>Smart</b> column align mode with additional space using selection')), 1, 2, 4, 5)

        table.attach(self.make_label(_('Toggle edit point')), 1, 2, 7, 8)
        table.attach(self.make_label(_('Add edit point at beginning of line/selection')), 1, 2, 8, 9)
        table.attach(self.make_label(_('Add edit point at end of line/selection')), 1, 2, 9, 10)
        table.attach(self.make_label(_('Align edit points')), 1, 2, 10, 11)
        table.attach(self.make_label(_('Align edit points with additional space')), 1, 2, 11, 12)

        table.show_all()
        tooltip.set_custom(table)
        return True

    def get_border_color(self):
        context = self._view.get_style_context()
        color = context.get_background_color(Gtk.StateFlags.NORMAL).copy()

        color.red = 1 - color.red
        color.green = 1 - color.green
        color.blue = 1 - color.blue
        color.alpha = 0.5

        return color

    def on_view_draw(self, view, cr):
        ##window = view.get_window(Gtk.TextWindowType.TEXT)

        #if Gtk.cairo_should_draw_window (cr, window):
        #    print("in 1st IF")
        #    return self._draw_column_mode(cr)

        window = view.get_window(Gtk.TextWindowType.TOP)

        #if window is None or not Gtk.cairo_should_draw_window (cr, window):
        #    print("in 2st IF")
        #    return False

        if window is None:
            print("in 2st IF")
            return False

        if not Gtk.cairo_should_draw_window (cr, window):
            print("in 2st NEW IF")
            return False

        if not self._in_mode:
            print("in 3st IF")
            return False

        print("hmmm")
        layout = self._view.create_pango_layout(_('Multi Edit Mode'))
        #layout = view.create_pango_layout(_('Multi Edit Mode'))
        extents = layout.get_pixel_extents()

        w = window.get_width()
        h = window.get_height()

        Gtk.cairo_transform_to_window(cr, view, window)

        cr.translate(0.5, 0.5)
        cr.set_line_width(1)

        col = self.get_border_color()
        Gdk.cairo_set_source_rgba(cr, col)

        cr.move_to(0, h - 1)
        cr.rel_line_to(w, 0)
        cr.stroke()

        context = self._view.get_style_context()
        Gdk.cairo_set_source_rgba(cr, context.get_color(Gtk.StateFlags.NORMAL))
        cr.move_to(w - extents[1].width - 3, (h - extents[1].height) / 2)
        PangoCairo.show_layout(cr, layout)

        if not self._status:
            status = ''
        else:
            status = str(self._status)

        if status:
            layout.set_markup(status, -1)

            cr.move_to(3, (h - extents[1].height) / 2)
            PangoCairo.show_layout(cr, layout)

        return False

    def on_smart_home_end(self, view, iter, count):
        if not self._in_mode or not self._edit_points or not self._previous_move_cursor:
            return

        ins = self._buffer.get_iter_at_mark(self._buffer.get_insert())

        if not ins.equal(iter):
            return

        if self._previous_move_cursor[0] == Gtk.MovementStep.DISPLAY_LINE_ENDS:
            cb = self._move_edit_point_smart_display_line_ends
        else:
            cb = self._move_edit_point_smart_paragraph_ends

        for point in self._edit_points:
            piter = self._buffer.get_iter_at_mark(point)
            piter = cb(piter, count)
            self._buffer.move_mark(point, piter)

    def on_move_cursor(self, view, step_size, count, extend_selection):
        self._move_cursor = [step_size, count]

    def on_move_cursor_after(self, view, step_size, count, extend_selection):
        self._move_cursor = None
        self._previous_move_cursor = None

# ex:ts=4:et:
