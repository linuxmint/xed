#    Pluma snippets plugin
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

import os
import weakref
import sys
import tempfile
import re

import gtk

import xml.etree.ElementTree as et
from Helper import *

class NamespacedId:
        def __init__(self, namespace, id):
                if not id:
                        self.id = None
                else:
                        if namespace:
                                self.id = namespace + '-'
                        else:
                                self.id = 'global-'
                
                        self.id += id

class SnippetData:
        PROPS = {'tag': '', 'text': '', 'description': 'New snippet', 
                        'accelerator': '', 'drop-targets': ''}

        def __init__(self, node, library):
                self.priv_id = node.attrib.get('id')

                self.set_library(library)
                self.valid = False
                self.set_node(node)

        def can_modify(self):
                return (self.library and (isinstance(self.library(), SnippetsUserFile)))

        def set_library(self, library):
                if library:
                        self.library = weakref.ref(library)
                else:
                        self.library = None

                self.id = NamespacedId(self.language(), self.priv_id).id

        def set_node(self, node):
                if self.can_modify():
                        self.node = node
                else:
                        self.node = None
                
                self.init_snippet_data(node)
                
        def init_snippet_data(self, node):
                if node == None:
                        return

                self.override = node.attrib.get('override')

                self.properties = {}
                props = SnippetData.PROPS.copy()

                # Store all properties present
                for child in node:
                        if child.tag in props:
                                del props[child.tag]

                                # Normalize accelerator
                                if child.tag == 'accelerator' and child.text != None:
                                        keyval, mod = gtk.accelerator_parse(child.text)

                                        if gtk.accelerator_valid(keyval, mod):
                                                child.text = gtk.accelerator_name(keyval, mod)
                                        else:
                                                child.text = ''

                                if self.can_modify():
                                        self.properties[child.tag] = child
                                else:
                                        self.properties[child.tag] = child.text or ''
                
                # Create all the props that were not found so we stay consistent
                for prop in props:
                        if self.can_modify():
                                child = et.SubElement(node, prop)

                                child.text = props[prop]
                                self.properties[prop] = child
                        else:
                                self.properties[prop] = props[prop]
                
                self.check_validation()
        
        def check_validation(self):
                if not self['tag'] and not self['accelerator'] and not self['drop-targets']:
                        return False

                library = Library()
                keyval, mod = gtk.accelerator_parse(self['accelerator'])
                
                self.valid = library.valid_tab_trigger(self['tag']) and \
                                (not self['accelerator'] or library.valid_accelerator(keyval, mod))
        
        def _format_prop(self, prop, value):
                if prop == 'drop-targets' and value != '':
                        return re.split('\\s*[,;]\\s*', value)
                else:
                        return value
        
        def __getitem__(self, prop):
                if prop in self.properties:
                        if self.can_modify():
                                return self._format_prop(prop, self.properties[prop].text or '')
                        else:
                                return self._format_prop(prop, self.properties[prop] or '')
                
                return self._format_prop(prop, '')
        
        def __setitem__(self, prop, value):
                if not prop in self.properties:
                        return
                
                if isinstance(value, list):
                        value = ','.join(value)
                               
                if not self.can_modify() and self.properties[prop] != value:
                        # ohoh, this is not can_modify, but it needs to be changed...
                        # make sure it is transfered to the changes file and set all the
                        # fields.
                        # This snippet data container will effectively become the container
                        # for the newly created node, but transparently to whoever uses
                        # it
                        self._override()

                if self.can_modify() and self.properties[prop].text != value:
                        if self.library():
                                self.library().tainted = True

                        oldvalue = self.properties[prop].text
                        self.properties[prop].text = value
                        
                        if prop == 'tag' or prop == 'accelerator' or prop == 'drop-targets':
                                container = Library().container(self.language())
                                container.prop_changed(self, prop, oldvalue)
                
                self.check_validation()

        def language(self):
                if self.library and self.library():
                        return self.library().language
                else:
                        return None
        
        def is_override(self):
                return self.override and Library().overridden[self.override]
        
        def to_xml(self):
                return self._create_xml()

        def _create_xml(self, parent=None, update=False, attrib={}):
                # Create a new node
                if parent != None:
                        element = et.SubElement(parent, 'snippet', attrib)
                else:
                        element = et.Element('snippet')

                # Create all the properties
                for p in self.properties:
                        prop = et.SubElement(element, p)
                        prop.text = self[p]
                        
                        if update:
                                self.properties[p] = prop
                
                return element              
        
        def _override(self):
                # Find the user file
                target = Library().get_user_library(self.language())

                # Create a new node there with override
                element = self._create_xml(target.root, True, {'override': self.id})

                # Create an override snippet data, feed it element so that it stores
                # all the values and then set the node to None so that it only contains
                # the values in .properties
                override = SnippetData(element, self.library())
                override.set_node(None)
                override.id = self.id
                
                # Set our node to the new element
                self.node = element
                
                # Set the override to our id
                self.override = self.id
                self.id = None
                
                # Set the new library
                self.set_library(target)
                
                # The library is tainted because we added this snippet
                target.tainted = True
                
                # Add the override
                Library().overridden[self.override] = override
        
        def revert(self, snippet):
                userlib = self.library()
                self.set_library(snippet.library())
                
                userlib.remove(self.node)
                
                self.set_node(None)

                # Copy the properties
                self.properties = snippet.properties
                
                # Set the id
                self.id = snippet.id

                # Reset the override flag
                self.override = None

class SnippetsTreeBuilder(et.TreeBuilder):
        def __init__(self, start=None, end=None):
                et.TreeBuilder.__init__(self)
                self.set_start(start)
                self.set_end(end)

        def set_start(self, start):
                self._start_cb = start
        
        def set_end(self, end):
                self._end_cb = end

        def start(self, tag, attrs):
                result = et.TreeBuilder.start(self, tag, attrs)
        
                if self._start_cb:
                        self._start_cb(result)
        
                return result
                
        def end(self, tag):
                result = et.TreeBuilder.end(self, tag)
        
                if self._end_cb:
                        self._end_cb(result)
        
                return result

class LanguageContainer:
        def __init__(self, language):
                self.language = language
                self.snippets = []
                self.snippets_by_prop = {'tag': {}, 'accelerator': {}, 'drop-targets': {}}
                self.accel_group = gtk.AccelGroup()
                self._refs = 0

        def _add_prop(self, snippet, prop, value=0):
                if value == 0:
                        value = snippet[prop]
                
                if not value or value == '':
                        return

                snippets_debug('Added ', prop ,' ', value, ' to ', str(self.language))
                
                if prop == 'accelerator':
                        keyval, mod = gtk.accelerator_parse(value)
                        self.accel_group.connect_group(keyval, mod, 0, \
                                        Library().accelerator_activated)
                
                snippets = self.snippets_by_prop[prop]
                
                if not isinstance(value, list):
                        value = [value]
                
                for val in value:
                        if val in snippets:
                                snippets[val].append(snippet)
                        else:
                                snippets[val] = [snippet]

        def _remove_prop(self, snippet, prop, value=0):
                if value == 0:
                        value = snippet[prop]

                if not value or value == '':
                        return

                snippets_debug('Removed ', prop, ' ', value, ' from ', str(self.language))

                if prop == 'accelerator':
                        keyval, mod = gtk.accelerator_parse(value)
                        self.accel_group.disconnect_key(keyval, mod)

                snippets = self.snippets_by_prop[prop]
                
                if not isinstance(value, list):
                        value = [value]
                
                for val in value:
                        try:
                                snippets[val].remove(snippet)
                        except:
                                True

        def append(self, snippet):
                tag = snippet['tag']
                accelerator = snippet['accelerator']
                
                self.snippets.append(snippet)
                
                self._add_prop(snippet, 'tag')
                self._add_prop(snippet, 'accelerator')
                self._add_prop(snippet, 'drop-targets')

                return snippet
        
        def remove(self, snippet):
                try:
                        self.snippets.remove(snippet)
                except:
                        True
                        
                self._remove_prop(snippet, 'tag')
                self._remove_prop(snippet, 'accelerator')
                self._remove_prop(snippet, 'drop-targets')
        
        def prop_changed(self, snippet, prop, oldvalue):
                snippets_debug('PROP CHANGED (', prop, ')', oldvalue)

                self._remove_prop(snippet, prop, oldvalue)
                self._add_prop(snippet, prop)
        
        def from_prop(self, prop, value):
                snippets = self.snippets_by_prop[prop]
                
                if prop == 'drop-targets':
                        s = []
                        
                        # FIXME: change this to use 
                        # matevfs.mime_type_get_equivalence when it comes
                        # available
                        for key, val in snippets.items():
                                if not value.startswith(key):
                                        continue
                                
                                for snippet in snippets[key]:
                                        if not snippet in s:
                                                s.append(snippet)
                        
                        return s
                else:
                        if value in snippets:
                                return snippets[value]
                        else:
                                return []
        
        def ref(self):
                self._refs += 1
        
                return True

        def unref(self):
                if self._refs > 0:
                        self._refs -= 1
                
                return self._refs != 0

class SnippetsSystemFile:
        def __init__(self, path=None):
                self.path = path
                self.loaded = False
                self.language = None
                self.ok = True
                self.need_id = True
                
        def load_error(self, message):
                sys.stderr.write("An error occurred loading " + self.path + ":\n")
                sys.stderr.write(message + "\nSnippets in this file will not be " \
                                "available, please correct or remove the file.\n")

        def _add_snippet(self, element):
                if not self.need_id or element.attrib.get('id'):
                        self.loading_elements.append(element)

        def set_language(self, element):
                self.language = element.attrib.get('language')
                
                if self.language:
                        self.language = self.language.lower()
        
        def _set_root(self, element):
                self.set_language(element)
                
        def _preprocess_element(self, element):
                if not self.loaded:
                        if not element.tag == "snippets":
                                self.load_error("Root element should be `snippets' instead " \
                                                "of `%s'" % element.tag)
                                return False
                        else:
                                self._set_root(element)
                                self.loaded = True
                elif element.tag != 'snippet' and not self.insnippet:
                        self.load_error("Element should be `snippet' instead of `%s'" \
                                        % element.tag)
                        return False
                else:
                        self.insnippet = True

                return True

        def _process_element(self, element):
                if element.tag == 'snippet':
                        self._add_snippet(element)
                        self.insnippet = False                        

                return True

        def ensure(self):
                if not self.ok or self.loaded:
                        return
                
                self.load()

        def parse_xml(self, readsize=16384):
                if not self.path:
                        return
                        
                elements = []

                builder = SnippetsTreeBuilder( \
                                lambda node: elements.append((node, True)), \
                                lambda node: elements.append((node, False)))

                parser = et.XMLTreeBuilder(target=builder)
                self.insnippet = False
                
                try:
                        f = open(self.path, "r")
                        
                        while True:
                                data = f.read(readsize)
                                
                                if not data:
                                        break
                                
                                parser.feed(data)
                                
                                for element in elements:
                                        yield element
                                
                                del elements[:]
                        
                        f.close()
                except IOError:
                        self.ok = False

        def load(self):
                if not self.ok:
                        return

                snippets_debug("Loading library (" + str(self.language) + "): " + \
                                self.path)
                
                self.loaded = False
                self.ok = False
                self.loading_elements = []
                
                for element in self.parse_xml():
                        if element[1]:
                                if not self._preprocess_element(element[0]):
                                        del self.loading_elements[:]
                                        return
                        else:
                                if not self._process_element(element[0]):
                                        del self.loading_elements[:]
                                        return

                for element in self.loading_elements:
                        snippet = Library().add_snippet(self, element)
                
                del self.loading_elements[:]
                self.ok = True

        # This function will get the language for a file by just inspecting the
        # root element of the file. This is provided so that a cache can be built
        # for which file contains which language.
        # It returns the name of the language
        def ensure_language(self):
                if not self.loaded:
                        self.ok = False
                        
                        for element in self.parse_xml(256):
                                if element[1]:
                                        if element[0].tag == 'snippets':
                                                self.set_language(element[0])
                                                self.ok = True

                                        break
        
        def unload(self):
                snippets_debug("Unloading library (" + str(self.language) + "): " + \
                                self.path)
                self.language = None
                self.loaded = False
                self.ok = True

class SnippetsUserFile(SnippetsSystemFile):
        def __init__(self, path=None):
                SnippetsSystemFile.__init__(self, path)
                self.tainted = False
                self.need_id = False
                
        def _set_root(self, element):
                SnippetsSystemFile._set_root(self, element)
                self.root = element
                        
        def add_prop(self, node, tag, data):
                if data[tag]:
                        prop = et.SubElement(node, tag)
                        prop.text = data[tag]
                
                        return prop
                else:
                        return None

        def new_snippet(self, properties=None):
                if (not self.ok) or self.root == None:
                        return None
                
                element = et.SubElement(self.root, 'snippet')
                
                if properties:
                        for prop in properties:
                                sub = et.SubElement(element, prop)
                                sub.text = properties[prop]
                
                self.tainted = True
                
                return Library().add_snippet(self, element)
        
        def set_language(self, element):
                SnippetsSystemFile.set_language(self, element)
                
                filename = os.path.basename(self.path).lower()
                
                if not self.language and filename == "global.xml":
                        self.modifier = True
                elif self.language and filename == self.language + ".xml":
                        self.modifier = True
                else:
                        self.modifier = False
        
        def create_root(self, language):
                if self.loaded:
                        snippets_debug('Not creating root, already loaded')
                        return
                
                if language:
                        root = et.Element('snippets', {'language': language})
                        self.path = os.path.join(Library().userdir, language.lower() + '.xml')
                else:
                        root = et.Element('snippets')
                        self.path = os.path.join(Library().userdir, 'global.xml')
                
                self._set_root(root)
                self.loaded = True
                self.ok = True
                self.tainted = True
                self.save()
        
        def remove(self, element):
                try:
                        self.root.remove(element)
                        self.tainted = True
                except:
                        return
                
                try:
                        first = self.root[0]
                except:
                        # No more elements, this library is useless now
                        Library().remove_library(self)
        
        def save(self):
                if not self.ok or self.root == None or not self.tainted:
                        return

                path = os.path.dirname(self.path)
                
                try:
                        if not os.path.isdir(path):
                                os.makedirs(path, 0755)
                except OSError:
                        # TODO: this is bad...
                        sys.stderr.write("Error in making dirs\n")

                try:
                        write_xml(self.root, self.path, ('text', 'accelerator'))
                        self.tainted = False
                except IOError:
                        # Couldn't save, what to do
                        sys.stderr.write("Could not save user snippets file to " + \
                                        self.path + "\n")
        
        def unload(self):
                SnippetsSystemFile.unload(self)
                self.root = None

class Singleton(object):
        _instance = None

        def __new__(cls, *args, **kwargs):
                if not cls._instance:
                        cls._instance = super(Singleton, cls).__new__(
                                         cls, *args, **kwargs)
                        cls._instance.__init_once__()

                return cls._instance

class Library(Singleton):        
        def __init_once__(self):
                self._accelerator_activated_cb = None
                self.loaded = False
                self.check_buffer = gtk.TextBuffer()

        def set_dirs(self, userdir, systemdirs):
                self.userdir = userdir
                self.systemdirs = systemdirs
                
                self.libraries = {}
                self.containers = {}
                self.overridden = {}
                self.loaded_ids = []

                self.loaded = False
        
        def set_accelerator_callback(self, cb):
                self._accelerator_activated_cb = cb
        
        def accelerator_activated(self, group, obj, keyval, mod):
                ret = False

                if self._accelerator_activated_cb:
                        ret = self._accelerator_activated_cb(group, obj, keyval, mod)

                return ret

        def add_snippet(self, library, element):
                container = self.container(library.language)
                overrided = self.overrided(library, element)
                
                if overrided:
                        overrided.set_library(library)
                        snippets_debug('Snippet is overriden: ' + overrided['description'])
                        return None
                
                snippet = SnippetData(element, library)
                
                if snippet.id in self.loaded_ids:
                        snippets_debug('Not added snippet ' + str(library.language) + \
                                        '::' + snippet['description'] + ' (duplicate)')
                        return None

                snippet = container.append(snippet)
                snippets_debug('Added snippet ' + str(library.language) + '::' + \
                                snippet['description'])
                
                if snippet and snippet.override:
                        self.add_override(snippet)
                
                if snippet.id:
                        self.loaded_ids.append(snippet.id)

                return snippet
        
        def container(self, language):
                language = self.normalize_language(language)
                
                if not language in self.containers:
                        self.containers[language] = LanguageContainer(language)
                
                return self.containers[language]
        
        def get_user_library(self, language):
                target = None
                
                if language in self.libraries:
                        for library in self.libraries[language]:
                                if isinstance(library, SnippetsUserFile) and library.modifier:
                                        target = library
                                elif not isinstance(library, SnippetsUserFile):
                                        break
                
                if not target:
                        # Create a new user file then
                        snippets_debug('Creating a new user file for language ' + \
                                        str(language))
                        target = SnippetsUserFile()
                        target.create_root(language)
                        self.add_library(target)
        
                return target
        
        def new_snippet(self, language, properties=None):
                language = self.normalize_language(language)
                library = self.get_user_library(language)

                return library.new_snippet(properties)
        
        def revert_snippet(self, snippet):
                # This will revert the snippet to the one it overrides
                if not snippet.can_modify() or not snippet.override in self.overridden:
                        # It can't be reverted, shouldn't happen, but oh..
                        return
                
                # The snippet in self.overriden only contains the property contents and
                # the library it belongs to
                revertto = self.overridden[snippet.override]
                del self.overridden[snippet.override]
                
                if revertto:
                        snippet.revert(revertto)
                
                        if revertto.id:
                                self.loaded_ids.append(revertto.id)
        
        def remove_snippet(self, snippet):
                if not snippet.can_modify() or snippet.is_override():
                        return
                
                # Remove from the library
                userlib = snippet.library()
                userlib.remove(snippet.node)
                
                # Remove from the container
                container = self.containers[userlib.language]
                container.remove(snippet)
        
        def overrided(self, library, element):
                id = NamespacedId(library.language, element.attrib.get('id')).id
                
                if id in self.overridden:
                        snippet = SnippetData(element, None)
                        snippet.set_node(None)
                        
                        self.overridden[id] = snippet
                        return snippet
                else:
                        return None
        
        def add_override(self, snippet):
                snippets_debug('Add override:', snippet.override)
                if not snippet.override in self.overridden:
                        self.overridden[snippet.override] = None
        
        def add_library(self, library):
                library.ensure_language()
                
                if not library.ok:
                        snippets_debug('Library in wrong format, ignoring')
                        return False
                
                snippets_debug('Adding library (' + str(library.language) + '): ' + \
                                library.path)

                if library.language in self.libraries:
                        # Make sure all the user files are before the system files
                        if isinstance(library, SnippetsUserFile):
                                self.libraries[library.language].insert(0, library)
                        else:
                                self.libraries[library.language].append(library)
                else:
                        self.libraries[library.language] = [library]

                return True
        
        def remove_library(self, library):
                if not library.ok:
                        return
                
                if library.path and os.path.isfile(library.path):
                        os.unlink(library.path)
                
                try:
                        self.libraries[library.language].remove(library)
                except KeyError:
                        True
                        
                container = self.containers[library.language]
                        
                for snippet in list(container.snippets):
                        if snippet.library() == library:
                                container.remove(snippet)
        
        def add_user_library(self, path):
                library = SnippetsUserFile(path)
                return self.add_library(library)
                
        def add_system_library(self, path):
                library = SnippetsSystemFile(path)
                return self.add_library(library)

        def find_libraries(self, path, searched, addcb):
                snippets_debug("Finding in: " + path)
                
                if not os.path.isdir(path):
                        return searched

                files = os.listdir(path)
                searched.append(path)
                
                for f in files:
                        f = os.path.realpath(os.path.join(path, f))

                        # Determine what language this file provides snippets for
                        if os.path.isfile(f):
                                addcb(f)
                
                return searched
        
        def normalize_language(self, language):
                if language:
                        return language.lower()
                
                return language
        
        def remove_container(self, language):
                for snippet in self.containers[language].snippets:
                        if snippet.id in self.loaded_ids:
                                self.loaded_ids.remove(snippet.id)

                        if snippet.override in self.overridden:
                                del self.overridden[snippet.override]

                del self.containers[language]
                
        def get_accel_group(self, language):
                language = self.normalize_language(language)
                container = self.container(language)

                self.ensure(language)
                return container.accel_group
                
        def save(self, language):
                language = self.normalize_language(language)
                
                if language in self.libraries:
                        for library in self.libraries[language]:
                                if isinstance(library, SnippetsUserFile):
                                        library.save()
                                else:
                                        break
        
        def ref(self, language):
                language = self.normalize_language(language)

                snippets_debug('Ref:', language)
                self.container(language).ref()
        
        def unref(self, language):
                language = self.normalize_language(language)
                
                snippets_debug('Unref:', language)
                
                if language in self.containers:
                        if not self.containers[language].unref() and \
                                        language in self.libraries:

                                for library in self.libraries[language]:
                                        library.unload()
                                
                                self.remove_container(language)

        def ensure(self, language):
                self.ensure_files()
                language = self.normalize_language(language)

                # Ensure language as well as the global snippets (None)
                for lang in (None, language):
                        if lang in self.libraries:
                                # Ensure the container exists
                                self.container(lang)

                                for library in self.libraries[lang]:
                                        library.ensure()

        def ensure_files(self):
                if self.loaded:
                        return

                searched = []
                searched = self.find_libraries(self.userdir, searched, \
                                self.add_user_library)
                
                for d in self.systemdirs:
                        searched = self.find_libraries(d, searched, \
                                        self.add_system_library)

                self.loaded = True

        def valid_accelerator(self, keyval, mod):
                mod &= gtk.accelerator_get_default_mod_mask()
        
                return (mod and (gdk.keyval_to_unicode(keyval) or \
                                keyval in range(gtk.keysyms.F1, gtk.keysyms.F12 + 1)))
        
        def valid_tab_trigger(self, trigger):
                if not trigger:
                        return True

                if trigger.isdigit():
                        return False

                self.check_buffer.set_text(trigger)

                start, end = self.check_buffer.get_bounds()
                text = self.check_buffer.get_text(start, end)
                                
                s = start.copy()
                e = end.copy()
                
                end.backward_word_start()
                start.forward_word_end()
                
                return (s.equal(end) and e.equal(start)) or (len(text) == 1 and not (text.isalnum() or text.isspace()))

        # Snippet getters
        # ===============
        def _from_prop(self, prop, value, language=None):
                self.ensure_files()
                
                result = []                
                language = self.normalize_language(language)
                        
                if not language in self.containers:
                        return []

                self.ensure(language)
                result = self.containers[language].from_prop(prop, value)
                
                if len(result) == 0 and language and None in self.containers:
                        result = self.containers[None].from_prop(prop, value)
                
                return result
        
        # Get snippets for a given language
        def get_snippets(self, language=None):
                self.ensure_files()
                language = self.normalize_language(language)
                
                if not language in self.libraries:
                        return []
                
                snippets = []
                self.ensure(language)
                
                return list(self.containers[language].snippets)

        # Get snippets for a given accelerator
        def from_accelerator(self, accelerator, language=None):
                return self._from_prop('accelerator', accelerator, language)

        # Get snippets for a given tag
        def from_tag(self, tag, language=None):
                return self._from_prop('tag', tag, language)
        
        # Get snippets for a given drop target
        def from_drop_target(self, drop_target, language=None):
                return self._from_prop('drop-targets', drop_target, language)
                
# ex:ts=8:et:
