#    Gedit snippets plugin
#    Copyright (C) 2006-2007  Jesse van den Kieboom <jesse@icecrew.nl>
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

import os
import re
import sys
from SubstitutionParser import SubstitutionParser

class Token:
        def __init__(self, klass, data):
                self.klass = klass
                self.data = data

        def __str__(self):
                return '%s: [%s]' % (self.klass, self.data)
                
        def __eq__(self, other):
                return self.klass == other.klass and self.data == other.data
        
        def __ne__(self, other):
                return not self.__eq__(other)

class Parser:
        SREG_ENV = '[A-Z_]+'
        SREG_ID = '[0-9]+'

        REG_ESCAPE = re.compile('(\\$(%s|\\(|\\{|<|%s)|`|\\\\)' % (SREG_ENV, SREG_ID))
        
        def __init__(self, **kwargs):
                for k, v in kwargs.items():
                        setattr(self, k, v)

                self.position = 0
                self.data_length = len(self.data)
                
                self.RULES = (self._match_env, self._match_regex, self._match_placeholder, self._match_shell, self._match_eval, self._text)
        
        def remains(self):
                return self.data[self.position:]

        def next_char(self):
                if self.position + 1 >= self.data_length:
                        return ''
                else:
                        return self.data[self.position + 1]
                
        def char(self):
                if self.position >= self.data_length:
                        return ''
                else:
                        return self.data[self.position]

        def token(self):
                self.tktext = ''

                while self.position < self.data_length:
                        try:
                                # Get first character
                                func = {'$': self._rule,
                                        '`': self._try_match_shell}[self.char()]
                        except:
                                func = self._text

                        # Detect end of text token
                        if func != self._text and self.tktext != '':
                                return Token('text', self.tktext)
                        
                        tk = func()

                        if tk:
                                return tk
                
                if self.tktext != '':
                        return Token('text', self.tktext)

        def _need_escape(self):
                text = self.remains()[1:]

                if text == '':
                        return False
                
                return self.REG_ESCAPE.match(text)
                
        def _escape(self):                
                if not self._need_escape():
                        return
                
                # Increase position with 1
                self.position += 1
                
        def _text(self):
                if self.char() == '\\':
                        self._escape()

                self.tktext += self.char()
                self.position += 1
        
        def _rule(self):
                for rule in self.RULES:
                        res = rule()
                        
                        if res:
                                return res

        def _match_env(self):
                text = self.remains()
                match = re.match('\\$(%s)' % self.SREG_ENV, text) or re.match('\\${(%s)}' % self.SREG_ENV, text)
                
                if match:
                        self.position += len(match.group(0))
                        return Token('environment', match.group(1))
        
        def _parse_list(self, lst):
                pos = 0
                length = len(lst)
                items = []
                last = None
                
                while pos < length:
                        char = lst[pos]
                        next = pos < length - 1 and lst[pos + 1]
                        
                        if char == '\\' and (next == ',' or next == ']'):
                                char = next
                                pos += 1
                        elif char == ',':
                                if last != None:
                                        items.append(last)
                                
                                last = None
                                pos += 1
                                continue

                        last = (last != None and last + char) or char
                        pos += 1
                
                if last != None:
                        items.append(last)
                
                return items
        
        def _parse_default(self, default):
                match = re.match('^\\s*(\\\\)?(\\[((\\\\]|[^\\]])+)\\]\\s*)$', default)
                
                if not match:
                        return [default]
                
                groups = match.groups()
                
                if groups[0]:
                        return [groups[1]]

                return self._parse_list(groups[2])
        
        def _match_placeholder(self):
                text = self.remains()
                
                match = re.match('\\${(%s)(:((\\\\\\}|[^}])+))?}' % self.SREG_ID, text) or re.match('\\$(%s)' % self.SREG_ID, text)
                
                if not match:
                        return None
                
                groups = match.groups()
                default = ''
                tabstop = int(groups[0])
                self.position += len(match.group(0))
                
                if len(groups) > 1 and groups[2]:
                        default = self._parse_default(groups[2].replace('\\}', '}'))

                return Token('placeholder', {'tabstop': tabstop, 'default': default})

        def _match_shell(self):
                text = self.remains()
                match = re.match('`((%s):)?((\\\\`|[^`])+?)`' % self.SREG_ID, text) or re.match('\\$\\(((%s):)?((\\\\\\)|[^\\)])+?)\\)' % self.SREG_ID, text)
                
                if not match:
                        return None
                
                groups = match.groups()
                tabstop = (groups[1] and int(groups[1])) or -1
                self.position += len(match.group(0))

                if text[0] == '`':
                        contents = groups[2].replace('\\`', '`')
                else:
                        contents = groups[2].replace('\\)', ')')
                
                return Token('shell', {'tabstop': tabstop, 'contents': contents})

        def _try_match_shell(self):
                return self._match_shell() or self._text()
        
        def _eval_options(self, options):
                reg = re.compile(self.SREG_ID)
                tabstop = -1
                depend = []
                
                options = options.split(':')
                
                for opt in options:
                        if reg.match(opt):
                                tabstop = int(opt)
                        else:
                                depend += self._parse_list(opt[1:-1])
                
                return (tabstop, depend)
                
        def _match_eval(self):
                text = self.remains()
                
                options = '((%s)|\\[([0-9, ]+)\\])' % self.SREG_ID
                match = re.match('\\$<((%s:)*)((\\\\>|[^>])+?)>' % options, text)
                
                if not match:
                        return None
                
                groups = match.groups()
                (tabstop, depend) = (groups[0] and self._eval_options(groups[0][:-1])) or (-1, [])
                self.position += len(match.group(0))
                
                return Token('eval', {'tabstop': tabstop, 'dependencies': depend, 'contents': groups[5].replace('\\>', '>')})
                
        def _match_regex(self):
                text = self.remains()
                
                content = '((?:\\\\[/]|\\\\}|[^/}])+)'
                match = re.match('\\${(?:(%s):)?\\s*(%s|\\$([A-Z_]+))?[/]%s[/]%s(?:[/]([a-zA-Z]*))?}' % (self.SREG_ID, self.SREG_ID, content, content), text)
                
                if not match:
                        return None
                
                groups = match.groups()
                tabstop = (groups[0] and int(groups[0])) or -1
                inp = (groups[2] or (groups[1] and int(groups[1]))) or ''
                
                pattern = re.sub('\\\\([/}])', '\\1', groups[3])
                substitution = re.sub('\\\\([/}])', '\\1', groups[4])
                modifiers = groups[5] or ''
                
                self.position += len(match.group(0))
                
                return Token('regex', {'tabstop': tabstop, 'input': inp, 'pattern': pattern, 'substitution': substitution, 'modifiers': modifiers})

# ex:ts=8:et:
