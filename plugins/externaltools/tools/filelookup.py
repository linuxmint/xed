# -*- coding: utf-8 -*-
#
#    Copyright (C) 2009-2010  Per Arneng <per.arneng@anyplanet.com>
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
import gio
import gedit

class FileLookup:
    """
    This class is responsible for looking up files given a part or the whole
    path of a real file. The lookup is delegated to providers wich use different
    methods of trying to find the real file.
    """

    def __init__(self):
        self.providers = []
        self.providers.append(AbsoluteFileLookupProvider())
        self.providers.append(CwdFileLookupProvider())
        self.providers.append(OpenDocumentRelPathFileLookupProvider())
        self.providers.append(OpenDocumentFileLookupProvider())

    def lookup(self, path):
        """
        Tries to find a file specified by the path parameter. It delegates to
        different lookup providers and the first match is returned. If no file
        was found then None is returned.

        path -- the path to find
        """
        found_file = None
        for provider in self.providers:
            found_file = provider.lookup(path)
            if found_file is not None:
                break

        return found_file


class FileLookupProvider:
    """
    The base class of all file lookup providers.
    """

    def lookup(self, path):
        """
        This method must be implemented by subclasses. Implementors will be
        given a path and will try to find a matching file. If no file is found
        then None is returned.
        """
        raise NotImplementedError("need to implement a lookup method")


class AbsoluteFileLookupProvider(FileLookupProvider):
    """
    This file tries to see if the path given is an absolute path and that the
    path references a file.
    """

    def lookup(self, path):
        if os.path.isabs(path) and os.path.isfile(path):
            return gio.File(path)
        else:
            return None


class CwdFileLookupProvider(FileLookupProvider):
    """
    This lookup provider tries to find a file specified by the path relative to
    the current working directory.
    """

    def lookup(self, path):
        try:
            cwd = os.getcwd()
        except OSError:
            cwd = os.getenv('HOME')

        real_path = os.path.join(cwd, path)

        if os.path.isfile(real_path):
            return gio.File(real_path)
        else:
            return None


class OpenDocumentRelPathFileLookupProvider(FileLookupProvider):
    """
    Tries to see if the path is relative to any directories where the
    currently open documents reside in. Example: If you have a document opened
    '/tmp/Makefile' and a lookup is made for 'src/test2.c' then this class
    will try to find '/tmp/src/test2.c'.
    """

    def lookup(self, path):
        if path.startswith('/'):
            return None

        for doc in gedit.app_get_default().get_documents():
            if doc.is_local():
                location = doc.get_location()
                if location:
                    rel_path = location.get_parent().get_path()
                    joined_path = os.path.join(rel_path, path)
                    if os.path.isfile(joined_path):
                        return gio.File(joined_path)

        return None


class OpenDocumentFileLookupProvider(FileLookupProvider):
    """
    Makes a guess that the if the path that was looked for matches the end
    of the path of a currently open document then that document is the one
    that is looked for. Example: If a document is opened called '/tmp/t.c'
    and a lookup is made for 't.c' or 'tmp/t.c' then both will match since
    the open document ends with the path that is searched for.
    """

    def lookup(self, path):
        if path.startswith('/'):
            return None

        for doc in gedit.app_get_default().get_documents():
            if doc.is_local():
                location = doc.get_location()
                if location and location.get_uri().endswith(path):
                    return location
        return None

# ex:ts=4:et:
