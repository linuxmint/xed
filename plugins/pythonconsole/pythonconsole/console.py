# -*- coding: utf-8 -*-

# pythonconsole.py -- Console widget
#
# Copyright (C) 2006 - Steve Frécinaux
# Copyright (C) 2012-2021 MATE Developers
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

# Parts from "Interactive Python-GTK Console" (stolen from epiphany's console.py)
#     Copyright (C), 1998 James Henstridge <james@daa.com.au>
#     Copyright (C), 2005 Adam Hooper <adamh@densi.com>
# Bits from xed Python Console Plugin
#     Copyrignt (C), 2005 Raphaël Slinckx

import string
import sys
import re
import traceback
from gi.repository import GObject, Gdk, Gtk, Pango

from .config import PythonConsoleConfig

__all__ = ('PythonConsole', 'OutFile')

class PythonConsole(Gtk.ScrolledWindow):

    __gsignals__ = {
        'grab-focus' : 'override',
    }

    DEFAULT_FONT = "Monospace 10"

    def __init__(self, namespace = {}):
        Gtk.ScrolledWindow.__init__(self)

        self.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        self.set_shadow_type(Gtk.ShadowType.IN)
        self.view = Gtk.TextView()
        self.view.set_editable(True)
        self.view.set_wrap_mode(Gtk.WrapMode.WORD_CHAR)
        self.add(self.view)
        self.view.show()

        buffer = self.view.get_buffer()
        self.normal = buffer.create_tag("normal")
        self.error  = buffer.create_tag("error")
        self.command = buffer.create_tag("command")

        self.config = PythonConsoleConfig()
        self.config.add_handler(self.apply_preferences)
        self.apply_preferences()

        self.__spaces_pattern = re.compile(r'^\s+')
        self.namespace = namespace

        self.block_command = False

        # Init first line
        buffer.create_mark("input-line", buffer.get_end_iter(), True)
        buffer.insert(buffer.get_end_iter(), ">>> ")
        buffer.create_mark("input", buffer.get_end_iter(), True)

        # Init history
        self.history = ['']
        self.history_pos = 0
        self.current_command = ''
        self.namespace['__history__'] = self.history

        # Set up hooks for standard output.
        self.stdout = OutFile(self, sys.stdout.fileno(), self.normal)
        self.stderr = OutFile(self, sys.stderr.fileno(), self.error)

        # Signals
        self.view.connect("key-press-event", self.__key_press_event_cb)
        buffer.connect("mark-set", self.__mark_set_cb)

    def do_grab_focus(self):
        self.view.grab_focus()

    def apply_preferences(self, *args):
        self.error.set_property("foreground", self.config.color_error)
        self.command.set_property("foreground", self.config.color_command)

        if self.config.use_system_font:
            font_name = self.config.monospace_font_name
        else:
            font_name = self.config.font

        font_desc = None
        try:
            font_desc = Pango.FontDescription(font_name)
        except:
            try:
                font_desc = Pango.FontDescription(self.config.monospace_font_name)
            except:
                try:
                    font_desc = Pango.FontDescription(self.DEFAULT_FONT)
                except:
                    pass

        if font_desc:
            self.view.modify_font(font_desc)

    def stop(self):
        self.namespace = None

    def __key_press_event_cb(self, view, event):
        modifier_mask = Gtk.accelerator_get_default_mod_mask()
        event_state = event.state & modifier_mask
        keyname = Gdk.keyval_name(event.keyval)

        if keyname == "d" and event_state == Gdk.ModifierType.CONTROL_MASK:
            self.destroy()

        elif keyname == "Return" and \
             event_state == Gdk.ModifierType.CONTROL_MASK:
            # Get the command
            buffer = view.get_buffer()
            inp_mark = buffer.get_mark("input")
            inp = buffer.get_iter_at_mark(inp_mark)
            cur = buffer.get_end_iter()
            line = buffer.get_text(inp, cur, False)
            self.current_command = self.current_command + line + "\n"
            self.history_add(line)

            # Prepare the new line
            cur = buffer.get_end_iter()
            buffer.insert(cur, "\n... ")
            cur = buffer.get_end_iter()
            buffer.move_mark(inp_mark, cur)

            # Keep indentation of precendent line
            spaces = re.match(self.__spaces_pattern, line)
            if spaces is not None:
                buffer.insert(cur, line[spaces.start() : spaces.end()])
                cur = buffer.get_end_iter()

            buffer.place_cursor(cur)
            GObject.idle_add(self.scroll_to_end)
            return True

        elif keyname == "Return":
            # Get the marks
            buffer = view.get_buffer()
            lin_mark = buffer.get_mark("input-line")
            inp_mark = buffer.get_mark("input")

            # Get the command line
            inp = buffer.get_iter_at_mark(inp_mark)
            cur = buffer.get_end_iter()
            line = buffer.get_text(inp, cur, False)
            self.current_command = self.current_command + line + "\n"
            self.history_add(line)

            # Make the line blue
            lin = buffer.get_iter_at_mark(lin_mark)
            buffer.apply_tag(self.command, lin, cur)
            buffer.insert(cur, "\n")

            cur_strip = self.current_command.rstrip()

            if cur_strip.endswith(":") \
            or (self.current_command[-2:] != "\n\n" and self.block_command):
                # Unfinished block command
                self.block_command = True
                com_mark = "... "
            elif cur_strip.endswith("\\"):
                com_mark = "... "
            else:
                # Eval the command
                self.__run(self.current_command)
                self.current_command = ''
                self.block_command = False
                com_mark = ">>> "

            # Prepare the new line
            cur = buffer.get_end_iter()
            buffer.move_mark(lin_mark, cur)
            buffer.insert(cur, com_mark)
            cur = buffer.get_end_iter()
            buffer.move_mark(inp_mark, cur)
            buffer.place_cursor(cur)
            GObject.idle_add(self.scroll_to_end)
            return True

        elif keyname == "KP_Down" or keyname == "Down":
            # Next entry from history
            view.emit_stop_by_name("key_press_event")
            self.history_down()
            GObject.idle_add(self.scroll_to_end)
            return True

        elif keyname == "KP_Up" or keyname == "Up":
            # Previous entry from history
            view.emit_stop_by_name("key_press_event")
            self.history_up()
            GObject.idle_add(self.scroll_to_end)
            return True

        elif keyname == "KP_Left" or keyname == "Left" or \
             keyname == "BackSpace":
            buffer = view.get_buffer()
            inp = buffer.get_iter_at_mark(buffer.get_mark("input"))
            cur = buffer.get_iter_at_mark(buffer.get_insert())
            if inp.compare(cur) == 0:
                if not event_state:
                    buffer.place_cursor(inp)
                return True
            return False

        # For the console we enable smart/home end behavior incoditionally
        # since it is useful when editing python

        elif (keyname == "KP_Home" or keyname == "Home") and \
             event_state == event_state & (Gdk.ModifierType.SHIFT_MASK|Gdk.ModifierType.CONTROL_MASK):
            # Go to the begin of the command instead of the begin of the line
            buffer = view.get_buffer()
            iter = buffer.get_iter_at_mark(buffer.get_mark("input"))
            ins = buffer.get_iter_at_mark(buffer.get_insert())

            while iter.get_char().isspace():
                iter.forward_char()

            if iter.equal(ins):
                iter = buffer.get_iter_at_mark(buffer.get_mark("input"))

            if event_state & Gdk.ModifierType.SHIFT_MASK:
                buffer.move_mark_by_name("insert", iter)
            else:
                buffer.place_cursor(iter)
            return True

        elif (keyname == "KP_End" or keyname == "End") and \
             event_state == event_state & (Gdk.ModifierType.SHIFT_MASK|Gdk.ModifierType.CONTROL_MASK):

            buffer = view.get_buffer()
            iter = buffer.get_end_iter()
            ins = buffer.get_iter_at_mark(buffer.get_insert())

            iter.backward_char()

            while iter.get_char().isspace():
                iter.backward_char()

            iter.forward_char()

            if iter.equal(ins):
                iter = buffer.get_end_iter()

            if event_state & Gdk.ModifierType.SHIFT_MASK:
                buffer.move_mark_by_name("insert", iter)
            else:
                buffer.place_cursor(iter)
            return True

    def __mark_set_cb(self, buffer, iter, name):
        input = buffer.get_iter_at_mark(buffer.get_mark("input"))
        pos   = buffer.get_iter_at_mark(buffer.get_insert())
        self.view.set_editable(pos.compare(input) != -1)

    def get_command_line(self):
        buffer = self.view.get_buffer()
        inp = buffer.get_iter_at_mark(buffer.get_mark("input"))
        cur = buffer.get_end_iter()
        return buffer.get_text(inp, cur, False)

    def set_command_line(self, command):
        buffer = self.view.get_buffer()
        mark = buffer.get_mark("input")
        inp = buffer.get_iter_at_mark(mark)
        cur = buffer.get_end_iter()
        buffer.delete(inp, cur)
        buffer.insert(inp, command)
        self.view.grab_focus()

    def history_add(self, line):
        if line.strip() != '':
            self.history_pos = len(self.history)
            self.history[self.history_pos - 1] = line
            self.history.append('')

    def history_up(self):
        if self.history_pos > 0:
            self.history[self.history_pos] = self.get_command_line()
            self.history_pos = self.history_pos - 1
            self.set_command_line(self.history[self.history_pos])

    def history_down(self):
        if self.history_pos < len(self.history) - 1:
            self.history[self.history_pos] = self.get_command_line()
            self.history_pos = self.history_pos + 1
            self.set_command_line(self.history[self.history_pos])

    def scroll_to_end(self):
        iter = self.view.get_buffer().get_end_iter()
        self.view.scroll_to_iter(iter, 0.0, False, 0.5, 0.5)
        return False

    def write(self, text, tag = None):
        buffer = self.view.get_buffer()
        if tag is None:
            buffer.insert(buffer.get_end_iter(), text)
        else:
            buffer.insert_with_tags(buffer.get_end_iter(), text, tag)
        GObject.idle_add(self.scroll_to_end)

    def eval(self, command, display_command = False):
        buffer = self.view.get_buffer()
        lin = buffer.get_mark("input-line")
        buffer.delete(buffer.get_iter_at_mark(lin),
                      buffer.get_end_iter())

        if isinstance(command, list) or isinstance(command, tuple):
            for c in command:
                if display_command:
                    self.write(">>> " + c + "\n", self.command)
                self.__run(c)
        else:
            if display_command:
                self.write(">>> " + c + "\n", self.command)
            self.__run(command)

        cur = buffer.get_end_iter()
        buffer.move_mark_by_name("input-line", cur)
        buffer.insert(cur, ">>> ")
        cur = buffer.get_end_iter()
        buffer.move_mark_by_name("input", cur)
        self.view.scroll_to_iter(buffer.get_end_iter(), 0.0, False, 0.5, 0.5)

    def __run(self, command):
        sys.stdout, self.stdout = self.stdout, sys.stdout
        sys.stderr, self.stderr = self.stderr, sys.stderr

        # eval and exec are broken in how they deal with utf8-encoded
        # strings so we have to explicitly decode the command before
        # passing it along
        try:
            command = command.decode('utf8')
        except:
            pass

        try:
            try:
                r = eval(command, self.namespace, self.namespace)
                if r is not None:
                    print(repr(r))
            except SyntaxError:
                exec(command, self.namespace)
        except:
            if hasattr(sys, 'last_type') and sys.last_type == SystemExit:
                self.destroy()
            else:
                traceback.print_exc()

        sys.stdout, self.stdout = self.stdout, sys.stdout
        sys.stderr, self.stderr = self.stderr, sys.stderr

    def destroy(self):
        pass
        #Gtk.ScrolledWindow.destroy(self)

class OutFile:
    """A fake output file object. It sends output to a TK test widget,
    and if asked for a file number, returns one set on instance creation"""
    def __init__(self, console, fn, tag):
        self.fn = fn
        self.console = console
        self.tag = tag
    def close(self):         pass
    def flush(self):         pass
    def fileno(self):        return self.fn
    def isatty(self):        return 0
    def read(self, a):       return ''
    def readline(self):      return ''
    def readlines(self):     return []
    def write(self, s):      self.console.write(s, self.tag)
    def writelines(self, l): self.console.write(l, self.tag)
    def seek(self, a):       raise IOError(29, 'Illegal seek')
    def tell(self):          raise IOError(29, 'Illegal seek')
    truncate = tell

# ex:et:ts=4:
