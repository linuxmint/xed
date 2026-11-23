# -*- coding: utf-8 -*-
#
#  bracketcompletion.py - Bracket completion plugin for Xed
#
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

import gi
gi.require_version('Peas', '1.0')
#gi.require_version('Xed', '3.0')
from gi.repository import GObject, Gdk, Xed

common_ident_brackets = {
    '(' : ')',
    '[' : ']',
    '{' : '}',
}

common_complete_brackets = {
    '"' : '"',
    "'" : "'",
}

language_brackets = {
    'changelog': { '<' : '>' },
    'html': { '<' : '>' },
    'ruby': { '|' : '|' },
    'sh': { '`' : '`' },
    'xml': { '<' : '>' },
    'php': { '<' : '>' },
}


class BracketCompletionPlugin(GObject.Object, Xed.ViewActivatable):
    __gtype_name__ = "BracketCompletion"
    view = GObject.Property(type=Xed.View)

    def __init__(self):
        GObject.Object.__init__(self)

    def do_activate(self):
        self._buffer = self.view.get_buffer()

        # State
        self._brackets = {}
        self._open_bracket_vals = {}
        self._close_bracket_vals = {}
        self._stack = []

        self._language_handler = self._buffer.connect('notify::language', self.on_notify_language)
        self._editable_handler = self.view.connect('notify::editable', self.on_notify_editable)
        self._key_handler = None

        self._try_activate()

    def do_deactivate(self):
        self._clear_state()

        if self._key_handler:
            self.view.disconnect(self._key_handler)
            self._key_handler = None
        if self._editable_handler:
            self.view.disconnect(self._editable_handler)
            self._editable_handler = None
        if self._language_handler:
            self._buffer.disconnect(self._language_handler)
            self._language_handler = None

    def on_notify_language(self, buffer, pspec):
        self._try_activate()

    def on_notify_editable(self, view, pspec):
        self._try_activate()


    def _clear_state(self):
        self._brackets.clear()
        self._open_bracket_vals.clear()
        self._close_bracket_vals.clear()
        self._stack.clear()

    def _try_activate(self):
        self._clear_state()
        
        lang = self._buffer.get_language()
        if lang:
            lang_id = lang.get_id()
            self._brackets.update(common_ident_brackets)
            self._brackets.update(common_complete_brackets)
            if lang_id in language_brackets:
                self._brackets.update(language_brackets[lang_id])

        for o, c in self._brackets.items():
            self._open_bracket_vals[Gdk.unicode_to_keyval(ord(o))] = o
            self._close_bracket_vals[Gdk.unicode_to_keyval(ord(c))] = c

        # Don't activate the feature if the buffer isn't editable or if
        # there are no brackets for the language
        active = self.view.get_editable() and self._brackets

        if active and self._key_handler is None:
            self._key_handler = self.view.connect('key-press-event', self.on_key_press_event)

        elif not active and self._key_handler is not None:
            self.view.disconnect(self._key_handler)
            self._key_handler = None


    def on_key_press_event(self, view, event):
        if event.state & (Gdk.ModifierType.CONTROL_MASK | Gdk.ModifierType.MOD1_MASK):
            return False

        if self._stack:
            if event.keyval in self._close_bracket_vals.keys():
                if self._try_consume_close(self._close_bracket_vals[event.keyval]):
                    return True
                # continue to process open_bracket
            
            if event.keyval == Gdk.KEY_BackSpace:
                return self._backspace()

            # navigation resets mode
            if event.keyval in (Gdk.KEY_Left, Gdk.KEY_Right, Gdk.KEY_Up, Gdk.KEY_Down):
                self._stack.clear()
                return False

        if event.keyval in (Gdk.KEY_Return, Gdk.KEY_KP_Enter):
            self._stack.clear()
            return self._auto_indent()

        if event.keyval in self._open_bracket_vals.keys():
            bounds = self._buffer.get_selection_bounds()
            if bounds:
                return self._wrap_bounds(self._open_bracket_vals[event.keyval], bounds[0], bounds[1])
            else:
                return self._auto_insert_close(self._open_bracket_vals[event.keyval])
        
        return False


    def _try_consume_close(self, key):
        assert self._stack

        if not self._buffer.get_selection_bounds():
            cursor = self._buffer.get_iter_at_mark(self._buffer.get_insert())

            if key == self._stack[-1] and key == cursor.get_char():
                cursor.forward_char()
                self._buffer.place_cursor(cursor)
                self._stack.pop()
                return True
        return False

    def _backspace(self):
        assert self._stack

        cursor = self._buffer.get_iter_at_mark(self._buffer.get_insert())
        cursor_char = cursor.get_char()

        if cursor_char == self._stack[-1]:
            start = cursor.copy()
            start.backward_char()
            last_char = start.get_char()

            if last_char in self._brackets and cursor_char == self._brackets[last_char]:
                end = cursor.copy()
                end.forward_char()
                self._buffer.delete(start, end)
                self._stack.pop()
                return True
        return False

    def _auto_indent(self):
        if self.view.get_auto_indent():
            cursor = self._buffer.get_iter_at_mark(self._buffer.get_insert())
            cursor_char = cursor.get_char()

            start = cursor.copy()
            start.backward_char()
            last_char = start.get_char()

            if last_char in common_ident_brackets and cursor_char == common_ident_brackets[last_char]:
                start.set_line_offset(0)

                cur_whitespace = []
                for c in start.get_visible_text(cursor):
                    if c == ' ' or c == '\t':
                        cur_whitespace.append(c)
                    else:
                        break
                cur_whitespace = "".join(cur_whitespace)

                use_spaces = True if cur_whitespace.startswith(' ') else self.view.get_insert_spaces_instead_of_tabs()
                addtl_indent = ' ' * self.view.get_tab_width() if use_spaces else '\t'

                self._buffer.insert(cursor, f'\n{cur_whitespace}{addtl_indent}\n{cur_whitespace}')

                cursor.backward_chars(len(cur_whitespace) + 1)
                self._buffer.place_cursor(cursor)
                return True
        return False

    def _wrap_bounds(self, o, start, end):
        c = self._brackets[o]

        self._buffer.begin_user_action()
        self._buffer.insert(end, c)

        # need to remove the inserted character from selection
        start, end = self._buffer.get_selection_bounds()
        end.backward_char()
        self._buffer.select_range(start, end)

        self._buffer.insert(start, o)
        self._buffer.end_user_action()

        self._stack.clear()
        return True

    def _auto_insert_close(self, o):
        cursor = self._buffer.get_iter_at_mark(self._buffer.get_insert())
        if not cursor.inside_word():
            c = self._brackets[o]
            self._buffer.insert(cursor, o + c)

            cursor.backward_char()
            self._buffer.place_cursor(cursor)
            self._stack.append(c)
            return True
        return False