# -*- coding: utf-8 -*-

# preprocessor.py - simple preprocessor for plugin template files
# This file is part of pluma
#
# Copyright (C) 2006 - Steve Frécinaux
#
# pluma is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# pluma is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with pluma; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, 
# Boston, MA  02110-1301  USA

import sys
import re

class DeepnessException(Exception):
    def __init__(self):
        Exception.__init__(self)

statements = [re.compile("^##\s*%s\s*$" % pattern) for pattern in
                        ['(?P<stmt>ifdef|ifndef)\s+(?P<key>[^\s]+)',
                         '(?P<stmt>elif|if)\s+(?P<expr>.+)',
                         '(?P<stmt>else|endif)',
                         '(?P<stmt>define)\s+(?P<key>[^\s]+)(\s+(?P<val>.+))?',
                         '(?P<stmt>undef)\s+(?P<key>[^\s]+)']]
variable = re.compile("##\((?P<name>[a-zA-Z_][a-zA-Z0-9_]*)(?P<mods>(\.[a-z]+)+)?\)")


def _eval(expr, macros):
    return eval(expr,
                {'defined': lambda x: macros.has_key(x)},
                macros)

def _subvar(match, macros):
    name = match.group('name')
    if name in macros:
        val = str(macros[name])
        if val is None:
            return ''
    else:
        return ''
    
    mods = match.group('mods')
    if mods is not None:
        for mod in mods[1:].split('.'):
            if mod == 'lower':
                val = val.lower()
            elif mod == 'upper':
                val = val.upper()
            elif mod == 'camel':
                val = ''.join(i.capitalize() 
                              for i in val.split('_'))
    return val

def process(infile = sys.stdin, outfile = sys.stdout, macros = {}):
    if not isinstance(infile, file):
        infile = open(infile, mode = 'r')
        close_infile = True
    else:
        close_infile = False

    if not isinstance(outfile, file):
        outfile = open(outfile, mode = 'w')
        close_outfile = True
    else:
        close_outfile = False
    
    deepness = 0
    writing_disabled = None
    
    for line in infile:
        # Skip comments
        if line[0:3].lower() == '##c':
            continue

        # Check whether current line is a preprocessor directive        
        for statement in statements:
            match = statement.match(line)
            if match: break
    
        if match is not None:
            stmt = match.group('stmt')

            if stmt == "define":
                if writing_disabled is None:
                    key = match.group('key')
                    val = match.group('val')
                    macros[key] = val

            elif stmt == "undef":
                if writing_disabled is None:
                    key = match.group('key')
                    if key in macros:
                        del macros[key]

            elif stmt == "ifdef":
                deepness += 1
                if writing_disabled is None and \
                   match.group('key') not in macros:
                    writing_disabled = deepness

            elif stmt == "ifndef":
                deepness += 1
                if writing_disabled is None and \
                   match.group('key') in macros:
                    writing_disabled = deepness                    

            elif stmt == "if":
                deepness += 1
                if writing_disabled is None and \
                   not _eval(match.group('expr'), macros):
                       writing_disabled = deepness

            elif stmt == "elif":
                if deepness == 0:
                    raise DeepnessException()
                if writing_disabled is None and \
                   not _eval(match.group('expr'), macros):
                    writing_disabled = deepness
                elif writing_disabled == deepness:
                    writing_disabled = None

            elif stmt == "else":
                if deepness == 0:
                    raise DeepnessException()
                if writing_disabled is None:
                    writing_disabled = deepness
                elif writing_disabled == deepness:
                    writing_disabled = None

            elif stmt == "endif":
                if deepness == 0:
                    raise DeepnessException()
                if writing_disabled is not None and \
                   writing_disabled == deepness:
                    writing_disabled = None
                deepness -= 1

        # Do variable substitution in the remaining lines
        elif writing_disabled is None:
            outfile.write(re.sub(variable, 
                                 lambda m: _subvar(m, macros),
                                 line))

    if deepness != 0:
        raise DeepnessException()

    if close_infile: infile.close()
    if close_outfile: outfile.close()

# ex:ts=4:et:
