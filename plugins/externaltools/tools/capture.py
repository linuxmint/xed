# -*- coding: utf-8 -*-
#    Gedit External Tools plugin
#    Copyright (C) 2005-2006  Steve Frécinaux <steve@istique.net>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

__all__ = ('Capture', )

import os, sys, signal
import locale
import subprocess
import gobject
import fcntl
import glib

class Capture(gobject.GObject):
    CAPTURE_STDOUT = 0x01
    CAPTURE_STDERR = 0x02
    CAPTURE_BOTH   = 0x03
    CAPTURE_NEEDS_SHELL = 0x04
    
    WRITE_BUFFER_SIZE = 0x4000

    __gsignals__ = {
        'stdout-line'  : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_STRING,)),
        'stderr-line'  : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_STRING,)),
        'begin-execute': (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, tuple()),
        'end-execute'  : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_INT,))
    }

    def __init__(self, command, cwd = None, env = {}):
        gobject.GObject.__init__(self)
        self.pipe = None
        self.env = env
        self.cwd = cwd
        self.flags = self.CAPTURE_BOTH | self.CAPTURE_NEEDS_SHELL
        self.command = command
        self.input_text = None

    def set_env(self, **values):
        self.env.update(**values)

    def set_command(self, command):
        self.command = command

    def set_flags(self, flags):
        self.flags = flags

    def set_input(self, text):
        self.input_text = text

    def set_cwd(self, cwd):
        self.cwd = cwd

    def execute(self):
        if self.command is None:
            return

        # Initialize pipe
        popen_args = {
            'cwd'  : self.cwd,
            'shell': self.flags & self.CAPTURE_NEEDS_SHELL,
            'env'  : self.env
        }
        
        if self.input_text is not None:
            popen_args['stdin'] = subprocess.PIPE
        if self.flags & self.CAPTURE_STDOUT:
            popen_args['stdout'] = subprocess.PIPE
        if self.flags & self.CAPTURE_STDERR:
            popen_args['stderr'] = subprocess.PIPE

        self.tried_killing = False
        self.idle_write_id = 0
        self.read_buffer = ''
        
        try:
            self.pipe = subprocess.Popen(self.command, **popen_args)
        except OSError, e:
            self.pipe = None
            self.emit('stderr-line', _('Could not execute command: %s') % (e, ))
            return
        
        # Signal
        self.emit('begin-execute')
        
        if self.flags & self.CAPTURE_STDOUT:
            # Set non blocking
            flags = fcntl.fcntl(self.pipe.stdout.fileno(), fcntl.F_GETFL) | os.O_NONBLOCK
            fcntl.fcntl(self.pipe.stdout.fileno(), fcntl.F_SETFL, flags)

            gobject.io_add_watch(self.pipe.stdout,
                                 gobject.IO_IN | gobject.IO_HUP,
                                 self.on_output)

        if self.flags & self.CAPTURE_STDERR:
            # Set non blocking
            flags = fcntl.fcntl(self.pipe.stderr.fileno(), fcntl.F_GETFL) | os.O_NONBLOCK
            fcntl.fcntl(self.pipe.stderr.fileno(), fcntl.F_SETFL, flags)

            gobject.io_add_watch(self.pipe.stderr,
                                 gobject.IO_IN | gobject.IO_HUP,
                                 self.on_output)

        # IO
        if self.input_text is not None:
            # Write async, in chunks of something
            self.write_buffer = str(self.input_text)

            if self.idle_write_chunk():
                self.idle_write_id = gobject.idle_add(self.idle_write_chunk)

        # Wait for the process to complete
        gobject.child_watch_add(self.pipe.pid, self.on_child_end)

    def idle_write_chunk(self):
        if not self.pipe:
            self.idle_write_id = 0
            return False

        try:
            l = len(self.write_buffer)
            m = min(l, self.WRITE_BUFFER_SIZE)
         
            self.pipe.stdin.write(self.write_buffer[:m])
            
            if m == l:
                self.write_buffer = ''
                self.pipe.stdin.close()
                
                self.idle_write_id = 0

                return False
            else:
                self.write_buffer = self.write_buffer[m:]
                return True
        except IOError:
            self.pipe.stdin.close()
            self.idle_write_id = 0

            return False

    def on_output(self, source, condition):
        if condition & (glib.IO_IN | glib.IO_PRI):
            line = source.read()

            if len(line) > 0:
                try:
                    line = unicode(line, 'utf-8')
                except:
                    line = unicode(line,
                                   locale.getdefaultlocale()[1],
                                   'replace')

                self.read_buffer += line
                lines = self.read_buffer.splitlines(True)
                
                if not lines[-1].endswith("\n"):
                    self.read_buffer = lines[-1]
                    lines = lines[0:-1]
                else:
                    self.read_buffer = ''

                for line in lines:
                    if not self.pipe or source == self.pipe.stdout:
                        self.emit('stdout-line', line)
                    else:
                        self.emit('stderr-line', line)

        if condition & ~(glib.IO_IN | glib.IO_PRI):
            if self.read_buffer:
                if source == self.pipe.stdout:
                    self.emit('stdout-line', self.read_buffer)
                else:
                    self.emit('stderr-line', self.read_buffer)

                self.read_buffer = ''

            self.pipe = None

            return False
        else:
            return True

    def stop(self, error_code = -1):
        if self.pipe is not None:
            if self.idle_write_id:
                gobject.source_remove(self.idle_write_id)
                self.idle_write_id = 0

            if not self.tried_killing:
                os.kill(self.pipe.pid, signal.SIGTERM)
                self.tried_killing = True
            else:
                os.kill(self.pipe.pid, signal.SIGKILL)

    def on_child_end(self, pid, error_code):
        # In an idle, so it is emitted after all the std*-line signals
        # have been intercepted
        gobject.idle_add(self.emit, 'end-execute', error_code)

# ex:ts=4:et:
