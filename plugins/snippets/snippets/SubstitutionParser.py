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

import re

class ParseError(Exception):
        def __str__(self):
                return 'Parse error, resume next'

class Modifiers:
        def _first_char(s):
                first = (s != '' and s[0]) or ''
                rest = (len(s) > 1 and s[1:]) or ''
                
                return first, rest
        
        def upper_first(s):
                first, rest = Modifiers._first_char(s)

                return '%s%s' % (first.upper(), rest)

        def upper(s):
                return s.upper()

        def lower_first(s):
                first, rest = Modifiers._first_char(s)

                return '%s%s' % (first.lower(), rest)
                
        def lower(s):
                return s.lower()
                
        def title(s):
                return s.title()
        
        upper_first = staticmethod(upper_first)
        upper = staticmethod(upper)
        lower_first = staticmethod(lower_first)
        lower = staticmethod(lower)
        title = staticmethod(title)
        _first_char = staticmethod(_first_char)

class SubstitutionParser:
        REG_ID = '[0-9]+'
        REG_NAME = '[a-zA-Z_]+'
        REG_MOD = '[a-zA-Z]+'
        REG_ESCAPE = '\\\\|\\(\\?|,|\\)'

        def __init__(self, pattern, groups = {}, modifiers = {}):
                self.pattern = pattern
                self.groups = groups
                
                self.REG_GROUP = '(?:(%s)|<(%s|%s)(?:,(%s))?>)' % (self.REG_ID, self.REG_ID, self.REG_NAME, self.REG_MOD)
                self.modifiers = {'u': Modifiers.upper_first,
                                  'U': Modifiers.upper,
                                  'l': Modifiers.lower_first,
                                  'L': Modifiers.lower,
                                  't': Modifiers.title}
                
                for k, v in modifiers.items():
                        self.modifiers[k] = v
        
        def parse(self):
                result, tokens = self._parse(self.pattern, None)
                
                return result
        
        def _parse(self, tokens, terminator):
                result = ''

                while tokens != '':
                        if self._peek(tokens) == '' or self._peek(tokens) == terminator:
                                tokens = self._remains(tokens)
                                break

                        try:
                                res, tokens = self._expr(tokens, terminator)
                        except ParseError:
                                res, tokens = self._text(tokens)
                        
                        result += res
                                
                return result, tokens
       
        def _peek(self, tokens, num = 0):
                return (num < len(tokens) and tokens[num])
        
        def _token(self, tokens):
                if tokens == '':
                        return '', '';

                return tokens[0], (len(tokens) > 1 and tokens[1:]) or ''
        
        def _remains(self, tokens, num = 1):
                return (num < len(tokens) and tokens[num:]) or ''

        def _expr(self, tokens, terminator):                
                if tokens == '':
                        return ''

                try:
                        return {'\\': self._escape,
                                '(': self._condition}[self._peek(tokens)](tokens, terminator)
                except KeyError:
                        raise ParseError

        def _text(self, tokens):
                return self._token(tokens)

        def _substitute(self, group, modifiers = ''):
                result = (self.groups.has_key(group) and self.groups[group]) or ''
                
                for modifier in modifiers:
                        if self.modifiers.has_key(modifier):
                                result = self.modifiers[modifier](result)
                
                return result
                
        def _match_group(self, tokens):
                match = re.match('\\\\%s' % self.REG_GROUP, tokens)
                
                if not match:
                        return None, tokens
                
                return self._substitute(match.group(1) or match.group(2), match.group(3) or ''), tokens[match.end():]
                
        def _escape(self, tokens, terminator):
                # Try to match a group
                result, tokens = self._match_group(tokens)
                
                if result != None:
                        return result, tokens
                
                s = self.REG_GROUP
                
                if terminator:
                        s += '|%s' % re.escape(terminator)

                match = re.match('\\\\(\\\\%s|%s)' % (s, self.REG_ESCAPE), tokens)

                if not match:
                        raise ParseError
                               
                return match.group(1), tokens[match.end():]  
        
        def _condition_value(self, tokens):
                match = re.match('\\\\?%s\s*' % self.REG_GROUP, tokens)
                
                if not match:
                        return None, tokens

                groups = match.groups()
                name = groups[0] or groups[1]

                return self.groups.has_key(name) and self.groups[name] != None, tokens[match.end():]        
        
        def _condition(self, tokens, terminator):
                # Match ? after (
                if self._peek(tokens, 1) != '?':
                        raise ParseError
               
                # Remove initial (? token
                tokens = self._remains(tokens, 2)
                condition, tokens = self._condition_value(tokens)
                
                if condition == None or self._peek(tokens) != ',':
                        raise ParseError

                truepart, tokens = self._parse(self._remains(tokens), ',')
                
                if truepart == None:
                        raise ParseError
                        
                falsepart, tokens = self._parse(tokens, ')')
                
                if falsepart == None:
                        raise ParseError

                if condition:
                        return truepart, tokens
                else:
                        return falsepart, tokens
        
        def escape_substitution(substitution):
                return re.sub('(%s|%s)' % (self.REG_GROUP, self.REG_ESCAPE), '\\\\\\1', substitution)
                
        escapesubstitution = staticmethod(escape_substitution)
# ex:ts=8:et:
