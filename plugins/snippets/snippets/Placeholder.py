#    Gedit snippets plugin
#    Copyright (C) 2005-2006  Jesse van den Kieboom <jesse@icecrew.nl>
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

import traceback
import re
import os
import sys
import signal
import select
import locale
import subprocess
from SubstitutionParser import SubstitutionParser
import gobject

from Helper import *

# These are places in a view where the cursor can go and do things
class Placeholder:
        def __init__(self, view, tabstop, defaults, begin):
                self.ok = True
                self.done = False
                self.buf = view.get_buffer()
                self.view = view
                self.has_references = False
                self.mirrors = []
                self.leave_mirrors = []
                self.tabstop = tabstop
                self.set_default(defaults)
                self.prev_contents = self.default
                self.set_mark_gravity()
                
                if begin:
                        self.begin = self.buf.create_mark(None, begin, self.mark_gravity[0])
                else:
                        self.begin = None
                
                self.end = None
        
        def __str__(self):
                return '%s (%s)' % (str(self.__class__), str(self.default))

        def set_mark_gravity(self):
                self.mark_gravity = [True, False]

        def set_default(self, defaults):
                self.default = None
                self.defaults = []

                if not defaults:
                        return

                for d in defaults:
                        dm = self.expand_environment(d)
                        
                        if dm:
                                self.defaults.append(dm)

                                if not self.default:
                                        self.default = dm
                                
                                if dm != d:
                                        break

        
        def literal(self, s):
                return repr(s)
                
        def format_environment(self, s):
                return s

        def re_environment(self, m):
                if m.group(1) or not m.group(2) in os.environ:
                        return '$' + m.group(2)
                else:
                        return self.format_environment(os.environ[m.group(2)])

        def expand_environment(self, text):
                if not text:
                        return text

                return re.sub('(\\\\)?\\$([A-Z_]+)', self.re_environment, text)
        
        def get_iter(self, mark):
                if mark and not mark.get_deleted():
                        return self.buf.get_iter_at_mark(mark)
                else:
                        return None

        def begin_iter(self):
                return self.get_iter(self.begin)
        
        def end_iter(self):
                return self.get_iter(self.end)
        
        def run_last(self, placeholders):
                begin = self.begin_iter()
                self.end = self.buf.create_mark(None, begin, self.mark_gravity[1])

                if self.default:
                        insert_with_indent(self.view, begin, self.default, False, self)
        
        def remove(self, force = False):
                if self.begin and not self.begin.get_deleted():
                        self.buf.delete_mark(self.begin)
                
                if self.end and not self.end.get_deleted():
                        self.buf.delete_mark(self.end)
                
        # Do something on beginning this placeholder
        def enter(self):
                if not self.begin or self.begin.get_deleted():
                        return

                self.buf.move_mark(self.buf.get_insert(), self.begin_iter())

                if self.end:
                        self.buf.move_mark(self.buf.get_selection_bound(), self.end_iter())
                else:
                        self.buf.move_mark(self.buf.get_selection_bound(), self.begin_iter())
        
        def get_text(self):
                if self.begin and self.end:
                        biter = self.begin_iter()
                        eiter = self.end_iter()
                        
                        if biter and eiter:
                                return self.buf.get_text(self.begin_iter(), self.end_iter())
                        else:
                                return ''
                else:
                        return ''
        
        def add_mirror(self, mirror, onleave = False):
                mirror.has_references = True

                if onleave:
                        self.leave_mirrors.append(mirror)
                else:
                        self.mirrors.append(mirror)

        def set_text(self, text):
                if self.begin.get_deleted() or self.end.get_deleted():
                        return

                # Set from self.begin to self.end to text!
                self.buf.begin_user_action()
                # Remove everything between self.begin and self.end
                begin = self.begin_iter()
                self.buf.delete(begin, self.end_iter())

                # Insert the text from the mirror
                insert_with_indent(self.view, begin, text, True, self)
                self.buf.end_user_action()
                
                self.update_contents()

        def update_contents(self):
                prev = self.prev_contents
                self.prev_contents = self.get_text()
                
                if prev != self.get_text():
                        for mirror in self.mirrors:
                                if not mirror.update(self):
                                        return

        def update_leave_mirrors(self):
                # Notify mirrors
                for mirror in self.leave_mirrors:
                        if not mirror.update(self):
                                return

        # Do something on ending this placeholder
        def leave(self):
               self.update_leave_mirrors()

        def find_mirrors(self, text, placeholders):
                mirrors = []
                
                while (True):
                        m = re.search('(\\\\)?\\$(?:{([0-9]+)}|([0-9]+))', text)
                        
                        if not m:
                                break
                        
                        # Skip escaped mirrors
                        if m.group(1):
                                text = text[m.end():]
                                continue

                        tabstop = int(m.group(2) or m.group(3))

                        if tabstop in placeholders:
                                if not tabstop in mirrors:
                                        mirrors.append(tabstop)

                                text = text[m.end():]
                        else:
                                self.ok = False
                                return None
                
                return mirrors 

# This is an placeholder which inserts a mirror of another Placeholder        
class PlaceholderMirror(Placeholder):
        def __init__(self, view, tabstop, begin):
                Placeholder.__init__(self, view, -1, None, begin)
                self.mirror_stop = tabstop

        def update(self, mirror):
                self.set_text(mirror.get_text())
                return True

        def run_last(self, placeholders):
                Placeholder.run_last(self, placeholders)

                if self.mirror_stop in placeholders:
                        mirror = placeholders[self.mirror_stop]
                        
                        mirror.add_mirror(self)
                        
                        if mirror.default:
                                self.set_text(mirror.default)
                else:
                        self.ok = False

# This placeholder indicates the end of a snippet
class PlaceholderEnd(Placeholder):
        def __init__(self, view, begin, default):
                Placeholder.__init__(self, view, 0, default, begin)
        
        def run_last(self, placeholders):
                Placeholder.run_last(self, placeholders)
                
                # Remove the begin mark and set the begin mark
                # to the end mark, this is needed so the end placeholder won't contain
                # any text
                
                if not self.default:
                        self.mark_gravity[0] = False
                        self.buf.delete_mark(self.begin)
                        self.begin = self.buf.create_mark(None, self.end_iter(), self.mark_gravity[0])

        def enter(self):
                if self.begin and not self.begin.get_deleted():
                        self.buf.move_mark(self.buf.get_insert(), self.begin_iter())
                
                if self.end and not self.end.get_deleted():
                        self.buf.move_mark(self.buf.get_selection_bound(), self.end_iter())
                
        def leave(self):
                self.enter()                        

# This placeholder is used to expand a command with embedded mirrors        
class PlaceholderExpand(Placeholder):
        def __init__(self, view, tabstop, begin, s):
                Placeholder.__init__(self, view, tabstop, None, begin)

                self.mirror_text = {0: ''}
                self.timeout_id = None
                self.cmd = s
                self.instant_update = False

        def __str__(self):
                s = Placeholder.__str__(self)
                
                return s + ' ' + self.cmd

        def get_mirrors(self, placeholders):
                return self.find_mirrors(self.cmd, placeholders)
                
        # Check if all substitution placeholders are accounted for
        def run_last(self, placeholders):
                Placeholder.run_last(self, placeholders)

                self.ok = True
                mirrors = self.get_mirrors(placeholders)
                
                if mirrors:
                        allDefault = True
                                
                        for mirror in mirrors:
                                p = placeholders[mirror]
                                p.add_mirror(self, not self.instant_update)
                                self.mirror_text[p.tabstop] = p.default
                                
                                if not p.default and not isinstance(p, PlaceholderExpand):
                                        allDefault = False
                        
                        if allDefault:
                                self.update(None)
                                self.default = self.get_text() or None
                else:
                        self.update(None)
                        self.default = self.get_text() or None

                        if self.tabstop == -1:
                                self.done = True
                
        def re_placeholder(self, m, formatter):
                if m.group(1):
                        return '"$' + m.group(2) + '"'
                else:
                        if m.group(3):
                                index = int(m.group(3))
                        else:
                                index = int(m.group(4))
                        
                        return formatter(self.mirror_text[index])

        def remove_timeout(self):
                if self.timeout_id != None:
                        gobject.source_remove(self.timeout_id)
                        self.timeout_id = None
                
        def install_timeout(self):
                self.remove_timeout()
                self.timeout_id = gobject.timeout_add(1000, self.timeout_cb)

        def timeout_cb(self):
                self.timeout_id = None
                
                return False
        
        def format_environment(self, text):
                return self.literal(text)

        def substitute(self, text, formatter = None):
                formatter = formatter or self.literal

                # substitute all mirrors, but also environmental variables
                text = re.sub('(\\\\)?\\$({([0-9]+)}|([0-9]+))', lambda m: self.re_placeholder(m, formatter), 
                                text)
                
                return self.expand_environment(text)
        
        def run_update(self):
                text = self.substitute(self.cmd)
                
                if text:
                        ret = self.expand(text)
                        
                        if ret:
                                self.update_leave_mirrors()
                else:
                        ret = True
                
                return ret
              
        def update(self, mirror):
                text = None
                
                if mirror:
                        self.mirror_text[mirror.tabstop] = mirror.get_text()
                        
                        # Check if all substitutions have been made
                        for tabstop in self.mirror_text:
                                if tabstop == 0:
                                        continue

                                if self.mirror_text[tabstop] == None:
                                        return False

                return self.run_update()

        def expand(self, text):
                return True

# The shell placeholder executes commands in a subshell
class PlaceholderShell(PlaceholderExpand):
        def __init__(self, view, tabstop, begin, s):
                PlaceholderExpand.__init__(self, view, tabstop, begin, s)

                self.shell = None
                self.remove_me = False

        def close_shell(self):
                self.shell.stdout.close()
                self.shell = None        
        
        def timeout_cb(self):
                PlaceholderExpand.timeout_cb(self)
                self.remove_timeout()
                
                if not self.shell:
                        return False

                gobject.source_remove(self.watch_id)
                self.close_shell()

                if self.remove_me:
                        PlaceholderExpand.remove(self)

                message_dialog(None, gtk.MESSAGE_ERROR, 'Execution of the shell ' \
                                'command (%s) exceeded the maximum time; ' \
                                'execution aborted.' % self.command)
                
                return False
        
        def process_close(self):
                self.close_shell()
                self.remove_timeout()

                self.set_text(str.join('', self.shell_output).rstrip('\n'))
                
                if self.default == None:
                        self.default = self.get_text()
                        self.leave()
                        
                if self.remove_me:
                        PlaceholderExpand.remove(self, True)
                
        def process_cb(self, source, condition):
                if condition & gobject.IO_IN:
                        line = source.readline()

                        if len(line) > 0:
                                try:
                                        line = unicode(line, 'utf-8')
                                except:
                                        line = unicode(line, locale.getdefaultlocale()[1], 
                                                        'replace')

                        self.shell_output += line
                        self.install_timeout()

                        return True

                self.process_close()
                return False
        
        def literal_replace(self, match):
                return "\\%s" % (match.group(0))

        def literal(self, text):
                return '"' + re.sub('([\\\\"])', self.literal_replace, text) + '"'
        
        def expand(self, text):
                self.remove_timeout()

                if self.shell:
                        gobject.source_remove(self.watch_id)
                        self.close_shell()

                popen_args = {
                        'cwd'  : None,
                        'shell': True,
                        'env'  : os.environ,
                        'stdout': subprocess.PIPE
                }

                self.command = text
                self.shell = subprocess.Popen(text, **popen_args)
                self.shell_output = ''
                self.watch_id = gobject.io_add_watch(self.shell.stdout, gobject.IO_IN | \
                                gobject.IO_HUP, self.process_cb)
                self.install_timeout()
                
                return True
                
        def remove(self, force = False):
                if not force and self.shell:
                        # Still executing shell command
                        self.remove_me = True
                else:
                        if force:
                                self.remove_timeout()
                                
                                if self.shell:
                                        self.close_shell()

                        PlaceholderExpand.remove(self, force)

class TimeoutError(Exception):
        def __init__(self, value):
                self.value = value
        
        def __str__(self):
                return repr(self.value)

# The python placeholder evaluates commands in python
class PlaceholderEval(PlaceholderExpand):
        def __init__(self, view, tabstop, refs, begin, s, namespace):
                PlaceholderExpand.__init__(self, view, tabstop, begin, s)

                self.fdread = 0
                self.remove_me = False
                self.namespace = namespace
                
                self.refs = []
                
                if refs:
                        for ref in refs:
                                self.refs.append(int(ref.strip()))

        def get_mirrors(self, placeholders):
                mirrors = PlaceholderExpand.get_mirrors(self, placeholders)

                if not self.ok:
                        return None

                for ref in self.refs:
                        if ref in placeholders:
                                if ref not in mirrors:
                                        mirrors.append(ref)
                        else:
                                self.ok = False
                                return None

                return mirrors

        # SIGALRM is not supported on all platforms (e.g. windows). Timeout
        # with SIGALRM will not be used on those platforms. This will
        # potentially block gedit if you have a placeholder which gets stuck,
        # but it's better than not supporting them at all. At some point we
        # might have proper thread support and we can fix this in a better way
        def timeout_supported(self):
                return hasattr(signal, 'SIGALRM')

        def timeout_cb(self, signum = 0, frame = 0):
                raise TimeoutError, "Operation timed out (>2 seconds)"
        
        def install_timeout(self):
                if not self.timeout_supported():
                        return

                if self.timeout_id != None:
                        self.remove_timeout()
                
                self.timeout_id = signal.signal(signal.SIGALRM, self.timeout_cb)
                signal.alarm(2)
                
        def remove_timeout(self):
                if not self.timeout_supported():
                        return

                if self.timeout_id != None:
                        signal.alarm(0)
                        
                        signal.signal(signal.SIGALRM, self.timeout_id)

                        self.timeout_id = None
                
        def expand(self, text):
                self.remove_timeout()

                text = text.strip()
                self.command = text

                if not self.command or self.command == '':
                        self.set_text('')
                        return

                text = "def process_snippet():\n\t" + "\n\t".join(text.split("\n"))
                
                if 'process_snippet' in self.namespace:
                        del self.namespace['process_snippet']

                try:
                        exec text in self.namespace
                except:
                        traceback.print_exc()

                if 'process_snippet' in self.namespace:
                        try:
                                # Install a sigalarm signal. This is a HACK to make sure 
                                # gedit doesn't get freezed by someone creating a python
                                # placeholder which for instance loops indefinately. Since
                                # the code is executed synchronously it will hang gedit. With
                                # the alarm signal we raise an exception and catch this
                                # (see below). We show an error message and return False.
                                # ___this is a HACK___ and should be fixed properly (I just 
                                # don't know how)                                
                                self.install_timeout()
                                result = self.namespace['process_snippet']()
                                self.remove_timeout()
                        except TimeoutError:
                                self.remove_timeout()

                                message_dialog(None, gtk.MESSAGE_ERROR, \
                                _('Execution of the Python command (%s) exceeds the maximum ' \
                                'time, execution aborted.') % self.command)
                                
                                return False
                        except Exception, detail:
                                self.remove_timeout()
                                
                                message_dialog(None, gtk.MESSAGE_ERROR, 
                                _('Execution of the Python command (%s) failed: %s') % 
                                (self.command, detail))

                                return False

                        if result == None:
                                # sys.stderr.write("%s:\n>> %s\n" % (_('The following python code, run in a snippet, does not return a value'), "\n>> ".join(self.command.split("\n"))))
                                result = ''

                        self.set_text(str(result))
                
                return True

# Regular expression placeholder
class PlaceholderRegex(PlaceholderExpand):
        def __init__(self, view, tabstop, begin, inp, pattern, substitution, modifiers):
                PlaceholderExpand.__init__(self, view, tabstop, begin, '')
                
                self.instant_update = True
                self.inp = inp
                self.pattern = pattern
                self.substitution = substitution
                
                self.init_modifiers(modifiers)
        
        def init_modifiers(self, modifiers):
                mods = {'I': re.I,
                        'L': re.L,
                        'M': re.M,
                        'S': re.S,
                        'U': re.U,
                        'X': re.X}
                
                self.modifiers = 0

                for modifier in modifiers:
                        if modifier in mods:
                                self.modifiers |= mods[modifier]

        def get_mirrors(self, placeholders):
                mirrors = self.find_mirrors(self.pattern, placeholders) + self.find_mirrors(self.substitution, placeholders)

                if isinstance(self.inp, int):
                        if self.inp not in placeholders:
                                self.ok = False
                                return None
                        elif self.inp not in mirrors:
                                mirrors.append(self.inp)

                return mirrors

        def literal(self, s):
                return re.escape(s)

        def get_input(self):
                if isinstance(self.inp, int):
                        return self.mirror_text[self.inp]
                elif self.inp in os.environ:
                        return os.environ[self.inp]
                else:
                        return ''
        
        def run_update(self):
                pattern = self.substitute(self.pattern)
                substitution = self.substitute(self.substitution, SubstitutionParser.escape_substitution)
                
                if pattern:
                        return self.expand(pattern, substitution)
                
                return True
        
        def expand(self, pattern, substitution):
                # Try to compile pattern
                try:
                        regex = re.compile(pattern, self.modifiers)
                except re.error, message:
                        sys.stderr.write('Could not compile regular expression: %s\n%s\n' % (pattern, message))
                        return False
                
                inp = self.get_input()
                match = regex.search(inp)
                
                if not match:
                        self.set_text(inp)
                else:
                        groups = match.groupdict()
                        
                        idx = 0
                        for group in match.groups():
                                groups[str(idx + 1)] = group
                                idx += 1

                        groups['0'] = match.group(0)

                        parser = SubstitutionParser(substitution, groups)
                        self.set_text(parser.parse())
                
                return True
# ex:ts=8:et:
