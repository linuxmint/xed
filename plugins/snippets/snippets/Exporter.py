import os
import tempfile
import sys
import shutil

from snippets.Library import *
import xml.etree.ElementTree as et
from Helper import *

class Exporter:
        def __init__(self, filename, snippets):
                self.filename = filename
                self.set_snippets(snippets)

        def set_snippets(self, snippets):
                self.snippets = {}
                
                for snippet in snippets:
                        lang = snippet.language()

                        if lang in self.snippets:
                                self.snippets[lang].append(snippet)
                        else:
                                self.snippets[lang] = [snippet]

        def export_xml(self, dirname, language, snippets):
                # Create the root snippets node
                root = et.Element('snippets')
                
                # Create filename based on language
                if language:
                        filename = os.path.join(dirname, language + '.xml')
                        
                        # Set the language attribute
                        root.attrib['language'] = language
                else:
                        filename = os.path.join(dirname, 'global.xml')
                
                # Add all snippets to the root node
                for snippet in snippets:
                        root.append(snippet.to_xml())
                
                # Write xml
                write_xml(root, filename, ('text', 'accelerator'))

        def export_archive(self, cmd):
                dirname = tempfile.mkdtemp()
                
                # Save current working directory and change to temporary directory
                curdir = os.getcwd()
                
                try:
                        os.chdir(dirname)
                
                        # Write snippet xml files
                        for language, snippets in self.snippets.items():
                                self.export_xml(dirname, language , snippets)
                
                        # Archive files
                        status = os.system('%s "%s" *.xml' % (cmd, self.filename))
                finally:
                        os.chdir(curdir)
                
                if status != 0:
                        return _('The archive "%s" could not be created' % self.filename)
                
                # Remove the temporary directory
                shutil.rmtree(dirname)

        def export_targz(self):
                self.export_archive('tar -c --gzip -f')
        
        def export_tarbz2(self):
                self.export_archive('tar -c --bzip2 -f')

        def export_tar(self):
                self.export_archive('tar -cf')
        
        def run(self):
                dirname = os.path.dirname(self.filename)
                if not os.path.exists(dirname):
                        return _('Target directory "%s" does not exist') % dirname
                
                if not os.path.isdir(dirname):
                        return _('Target directory "%s" is not a valid directory') % dirname
                
                (root, ext) = os.path.splitext(self.filename)
                
                actions = {'.tar.gz': self.export_targz,
                           '.tar.bz2': self.export_tarbz2,
                           '.tar': self.export_tar}

                for k, v in actions.items():
                        if self.filename.endswith(k):
                                return v()
                        
                return self.export_targz()
# ex:ts=8:et:
