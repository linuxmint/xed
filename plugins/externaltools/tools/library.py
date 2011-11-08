# -*- coding: utf-8 -*-
#    Pluma External Tools plugin
#    Copyright (C) 2006  Steve Frécinaux <code@istique.net>
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
import locale
import platform

class Singleton(object):
    _instance = None

    def __new__(cls, *args, **kwargs):
        if not cls._instance:
            cls._instance = super(Singleton, cls).__new__(
                             cls, *args, **kwargs)
            cls._instance.__init_once__()

        return cls._instance

class ToolLibrary(Singleton):
    def __init_once__(self):
        self.locations = []

    def set_locations(self, datadir):
        self.locations = []

        if platform.platform() != 'Windows':
            for d in self.get_xdg_data_dirs():
                self.locations.append(os.path.join(d, 'pluma', 'plugins', 'externaltools', 'tools'))

        self.locations.append(datadir)

        # self.locations[0] is where we save the custom scripts
        if platform.platform() == 'Windows':
            toolsdir = os.path.expanduser('~/pluma/tools')
        else:
            userdir = os.getenv('MATE22_USER_DIR')
            if userdir:
                toolsdir = os.path.join(userdir, 'pluma/tools')
            else:
                toolsdir = os.path.expanduser('~/.config/pluma/tools')

        self.locations.insert(0, toolsdir);

        if not os.path.isdir(self.locations[0]):
            os.makedirs(self.locations[0])
            self.tree = ToolDirectory(self, '')
            self.import_old_xml_store()
        else:
            self.tree = ToolDirectory(self, '')

    # cf. http://standards.freedesktop.org/basedir-spec/latest/
    def get_xdg_data_dirs(self):
        dirs = os.getenv('XDG_DATA_DIRS')
        if dirs:
            dirs = dirs.split(os.pathsep)
        else:
            dirs = ('/usr/local/share', '/usr/share')
        return dirs

    # This function is meant to be ran only once, when the tools directory is
    # created. It imports eventual tools that have been saved in the old XML
    # storage file.
    def import_old_xml_store(self):
        import xml.etree.ElementTree as et
        userdir = os.getenv('MATE22_USER_DIR')
        if userdir:
            filename = os.path.join(userdir, 'pluma/pluma-tools.xml')
        else:
            filename = os.path.expanduser('~/.config/pluma/pluma-tools.xml')

        if not os.path.isfile(filename):
            return

        print "External tools: importing old tools into the new store..."

        xtree = et.parse(filename)
        xroot = xtree.getroot()

        for xtool in xroot:
            for i in self.tree.tools:
                if i.name == xtool.get('label'):
                    tool = i
                    break
            else:
                tool = Tool(self.tree)
                tool.name = xtool.get('label')
                tool.autoset_filename()
                self.tree.tools.append(tool)
            tool.comment = xtool.get('description')
            tool.shortcut = xtool.get('accelerator')
            tool.applicability = xtool.get('applicability')
            tool.output = xtool.get('output')
            tool.input = xtool.get('input')

            tool.save_with_script(xtool.text)

    def get_full_path(self, path, mode='r', system = True, local = True):
        assert (system or local)
        if path is None:
            return None
        if mode == 'r':
            if system and local:
                locations = self.locations
            elif local and not system:
                locations = [self.locations[0]]
            elif system and not local:
                locations = self.locations[1:]
            else:
                raise ValueError("system and local can't be both set to False")

            for i in locations:
                p = os.path.join(i, path)
                if os.path.lexists(p):
                    return p
            return None
        else:
            path = os.path.join(self.locations[0], path)
            dirname = os.path.dirname(path)
            if not os.path.isdir(dirname):
                os.mkdir(dirname)
            return path

class ToolDirectory(object):
    def __init__(self, parent, dirname):
        super(ToolDirectory, self).__init__()
        self.subdirs = list()
        self.tools = list()
        if isinstance(parent, ToolDirectory):
            self.parent = parent
            self.library = parent.library
        else:
            self.parent = None
            self.library = parent
        self.dirname = dirname
        self._load()

    def listdir(self):
        elements = dict()
        for l in self.library.locations:
            d = os.path.join(l, self.dirname)
            if not os.path.isdir(d):
                continue
            for i in os.listdir(d):
                elements[i] = None
        keys = elements.keys()
        keys.sort()
        return keys

    def _load(self):
        for p in self.listdir():
            path = os.path.join(self.dirname, p)
            full_path = self.library.get_full_path(path)
            if os.path.isdir(full_path):
                self.subdirs.append(ToolDirectory(self, p))
            elif os.path.isfile(full_path) and os.access(full_path, os.X_OK):
                self.tools.append(Tool(self, p))

    def get_path(self):
        if self.parent is None:
            return self.dirname
        else:
            return os.path.join(self.parent.get_path(), self.dirname)
    path = property(get_path)

    def get_name(self):
        return os.path.basename(self.dirname)
    name = property(get_name)

    def delete_tool(self, tool):
        # Only remove it if it lays in $HOME
        if tool in self.tools:
            path = tool.get_path()
            if path is not None:
                filename = os.path.join(self.library.locations[0], path)
                if os.path.isfile(filename):
                    os.unlink(filename)
            self.tools.remove(tool)
            return True
        else:
            return False

    def revert_tool(self, tool):
        # Only remove it if it lays in $HOME
        filename = os.path.join(self.library.locations[0], tool.get_path())
        if tool in self.tools and os.path.isfile(filename):
            os.unlink(filename)
            tool._load()
            return True
        else:
            return False


class Tool(object):
    RE_KEY = re.compile('^([a-zA-Z_][a-zA-Z0-9_.\-]*)(\[([a-zA-Z_@]+)\])?$')

    def __init__(self, parent, filename = None):
        super(Tool, self).__init__()
        self.parent = parent
        self.library = parent.library
        self.filename = filename
        self.changed = False
        self._properties = dict()
        self._transform = {
            'Languages': [self._to_list, self._from_list]
        }
        self._load()

    def _to_list(self, value):
        if value.strip() == '':
            return []
        else:
            return map(lambda x: x.strip(), value.split(','))

    def _from_list(self, value):
        return ','.join(value)

    def _parse_value(self, key, value):
        if key in self._transform:
            return self._transform[key][0](value)
        else:
            return value

    def _load(self):
        if self.filename is None:
            return

        filename = self.library.get_full_path(self.get_path())
        if filename is None:
            return

        fp = file(filename, 'r', 1)
        in_block = False
        lang = locale.getlocale(locale.LC_MESSAGES)[0]

        for line in fp:
            if not in_block:
                in_block = line.startswith('# [Pluma Tool]')
                continue
            if line.startswith('##') or line.startswith('# #'): continue
            if not line.startswith('# '): break

            try:
                (key, value) = [i.strip() for i in line[2:].split('=', 1)]
                m = self.RE_KEY.match(key)
                if m.group(3) is None:
                    self._properties[m.group(1)] = self._parse_value(m.group(1), value)
                elif lang is not None and lang.startswith(m.group(3)):
                    self._properties[m.group(1)] = self._parse_value(m.group(1), value)
            except ValueError:
                break
        fp.close()
        self.changed = False

    def _set_property_if_changed(self, key, value):
        if value != self._properties.get(key):
            self._properties[key] = value

            self.changed = True

    def is_global(self):
        return self.library.get_full_path(self.get_path(), local=False) is not None

    def is_local(self):
        return self.library.get_full_path(self.get_path(), system=False) is not None

    def is_global(self):
        return self.library.get_full_path(self.get_path(), local=False) is not None

    def get_path(self):
        if self.filename is not None:
            return os.path.join(self.parent.get_path(), self.filename)
        else:
            return None
    path = property(get_path)

    # This command is the one that is meant to be ran
    # (later, could have an Exec key or something)
    def get_command(self):
        return self.library.get_full_path(self.get_path())
    command = property(get_command)

    def get_applicability(self):
        applicability = self._properties.get('Applicability')
        if applicability: return applicability
        return 'all'
    def set_applicability(self, value):
        self._set_property_if_changed('Applicability', value)
    applicability = property(get_applicability, set_applicability)

    def get_name(self):
        name = self._properties.get('Name')
        if name: return name
        return os.path.basename(self.filename)
    def set_name(self, value):
        self._set_property_if_changed('Name', value)
    name = property(get_name, set_name)

    def get_shortcut(self):
        shortcut = self._properties.get('Shortcut')
        if shortcut: return shortcut
        return None
    def set_shortcut(self, value):
        self._set_property_if_changed('Shortcut', value)
    shortcut = property(get_shortcut, set_shortcut)

    def get_comment(self):
        comment = self._properties.get('Comment')
        if comment: return comment
        return self.filename
    def set_comment(self, value):
        self._set_property_if_changed('Comment', value)
    comment = property(get_comment, set_comment)

    def get_input(self):
        input = self._properties.get('Input')
        if input: return input
        return 'nothing'
    def set_input(self, value):
        self._set_property_if_changed('Input', value)
    input = property(get_input, set_input)

    def get_output(self):
        output = self._properties.get('Output')
        if output: return output
        return 'output-panel'
    def set_output(self, value):
        self._set_property_if_changed('Output', value)
    output = property(get_output, set_output)

    def get_save_files(self):
        save_files = self._properties.get('Save-files')
        if save_files: return save_files
        return 'nothing'
    def set_save_files(self, value):
        self._set_property_if_changed('Save-files', value)
    save_files = property(get_save_files, set_save_files)

    def get_languages(self):
        languages = self._properties.get('Languages')
        if languages: return languages
        return []
    def set_languages(self, value):
        self._set_property_if_changed('Languages', value)
    languages = property(get_languages, set_languages)

    def has_hash_bang(self):
        if self.filename is None:
            return True

        filename = self.library.get_full_path(self.get_path())
        if filename is None:
            return True

        fp = open(filename, 'r', 1)
        for line in fp:
            if line.strip() == '':
                continue

            return line.startswith('#!')

    # There is no property for this one because this function is quite
    # expensive to perform
    def get_script(self):
        if self.filename is None:
            return ["#!/bin/sh\n"]

        filename = self.library.get_full_path(self.get_path())
        if filename is None:
            return ["#!/bin/sh\n"]

        fp = open(filename, 'r', 1)
        lines = list()

        # before entering the data block
        for line in fp:
            if line.startswith('# [Pluma Tool]'):
                break
            lines.append(line)
        # in the block:
        for line in fp:
            if line.startswith('##'): continue
            if not (line.startswith('# ') and '=' in line):
                # after the block: strip one emtpy line (if present)
                if line.strip() != '':
                    lines.append(line)
                break
        # after the block
        for line in fp:
            lines.append(line)
        fp.close()
        return lines

    def _dump_properties(self):
        lines = ['# [Pluma Tool]']
        for item in self._properties.iteritems():
            if item[0] in self._transform:
                lines.append('# %s=%s' % (item[0], self._transform[item[0]][1](item[1])))
            elif item[1] is not None:
                lines.append('# %s=%s' % item)
        return '\n'.join(lines) + '\n'

    def save_with_script(self, script):
        filename = self.library.get_full_path(self.filename, 'w')

        fp = open(filename, 'w', 1)

        # Make sure to first print header (shebang, modeline), then
        # properties, and then actual content
        header = []
        content = []
        inheader = True

        # Parse
        for line in script:
            line = line.rstrip("\n")

            if not inheader:
                content.append(line)
            elif line.startswith('#!'):
                # Shebang (should be always present)
                header.append(line)
            elif line.strip().startswith('#') and ('-*-' in line or 'ex:' in line or 'vi:' in line or 'vim:' in line):
                header.append(line)
            else:
                content.append(line)
                inheader = False

        # Write out header
        for line in header:
            fp.write(line + "\n")

        fp.write(self._dump_properties())
        fp.write("\n")

        for line in content:
            fp.write(line + "\n")

        fp.close()
        os.chmod(filename, 0750)
        self.changed = False

    def save(self):
        if self.changed:
            self.save_with_script(self.get_script())

    def autoset_filename(self):
        if self.filename is not None:
            return
        dirname = self.parent.path
        if dirname != '':
            dirname += os.path.sep

        basename = self.name.lower().replace(' ', '-').replace('/', '-')

        if self.library.get_full_path(dirname + basename):
            i = 2
            while self.library.get_full_path(dirname + "%s-%d" % (basename, i)):
                i += 1
            basename = "%s-%d" % (basename, i)
        self.filename = basename

if __name__ == '__main__':
    library = ToolLibrary()

    def print_tool(t, indent):
        print indent * "  " + "%s: %s" % (t.filename, t.name)

    def print_dir(d, indent):
        print indent * "  " + d.dirname + '/'
        for i in d.subdirs:
            print_dir(i, indent+1)
        for i in d.tools:
            print_tool(i, indent+1)

    print_dir(library.tree, 0)

# ex:ts=4:et:
