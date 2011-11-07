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

import re

class Link:
    """
    This class represents a file link from within a string given by the
    output of some software tool. A link contains a reference to a file, the
    line number within the file and the boundaries within the given output
    string that should be marked as a link.
    """

    def __init__(self, path, line_nr, start, end):
        """
        path -- the path of the file (that could be extracted)
        line_nr -- the line nr of the specified file
        start -- the index within the string that the link starts at
        end -- the index within the string where the link ends at
        """
        self.path    = path
        self.line_nr = int(line_nr)
        self.start   = start
        self.end     = end

    def __repr__(self):
        return "%s[%s](%s:%s)" % (self.path, self.line_nr,
                                  self.start, self.end)

class LinkParser:
    """
    Parses a text using different parsing providers with the goal of finding one
    or more file links within the text. A typical example could be the output
    from a compiler that specifies an error in a specific file. The path of the
    file, the line nr and some more info is then returned so that it can be used
    to be able to navigate from the error output in to the specific file.

    The actual work of parsing the text is done by instances of classes that
    inherits from AbstractLinkParser or by regular expressions. To add a new
    parser just create a class that inherits from AbstractLinkParser and then
    register in this class cunstructor using the method add_parser. If you want
    to add a regular expression then just call add_regexp in this class
    constructor and provide your regexp string as argument.
    """

    def __init__(self):
        self._providers = []
        self.add_regexp(REGEXP_STANDARD)
        self.add_regexp(REGEXP_PYTHON)
        self.add_regexp(REGEXP_VALAC)
        self.add_regexp(REGEXP_BASH)
        self.add_regexp(REGEXP_RUBY)
        self.add_regexp(REGEXP_PERL)
        self.add_regexp(REGEXP_MCS)

    def add_parser(self, parser):
        self._providers.append(parser)

    def add_regexp(self, regexp):
        """
        Adds a regular expression string that should match a link using
        re.MULTILINE and re.VERBOSE regexp. The area marked as a link should
        be captured by a group named lnk. The path of the link should be
        captured by a group named pth. The line number should be captured by
        a group named ln. To read more about this look at the documentation
        for the RegexpLinkParser constructor.
        """
        self.add_parser(RegexpLinkParser(regexp))

    def parse(self, text):
        """
        Parses the given text and returns a list of links that are parsed from
        the text. This method delegates to parser providers that can parse
        output from different kinds of formats. If no links are found then an
        empty list is returned.

        text -- the text to scan for file links. 'text' can not be None.
        """
        if text is None:
            raise ValueError("text can not be None")

        links = []

        for provider in self._providers:
            links.extend(provider.parse(text))

        return links

class AbstractLinkParser(object):
    """The "abstract" base class for link parses"""

    def parse(self, text):
        """
        This method should be implemented by subclasses. It takes a text as
        argument (never None) and then returns a list of Link objects. If no
        links are found then an empty list is expected. The Link class is
        defined in this module. If you do not override this method then a
        NotImplementedError will be thrown.

        text -- the text to parse. This argument is never None.
        """
        raise NotImplementedError("need to implement a parse method")

class RegexpLinkParser(AbstractLinkParser):
    """
    A class that represents parsers that only use one single regular expression.
    It can be used by subclasses or by itself. See the constructor documentation
    for details about the rules surrouning the regexp.
    """

    def __init__(self, regex):
        """
        Creates a new RegexpLinkParser based on the given regular expression.
        The regular expression is multiline and verbose (se python docs on
        compilation flags). The regular expression should contain three named
        capturing groups 'lnk', 'pth' and 'ln'. 'lnk' represents the area wich
        should be marked as a link in the text. 'pth' is the path that should
        be looked for and 'ln' is the line number in that file.
        """
        self.re = re.compile(regex, re.MULTILINE | re.VERBOSE)

    def parse(self, text):
        links = []
        for m in re.finditer(self.re, text):
            path = m.group("pth")
            line_nr = m.group("ln")
            start = m.start("lnk")
            end = m.end("lnk")
            link = Link(path, line_nr, start, end)
            links.append(link)

        return links

# gcc 'test.c:13: warning: ...'
# javac 'Test.java:13: ...'
# ruby 'test.rb:5: ...'
# scalac 'Test.scala:5: ...'
# 6g (go) 'test.go:9: ...'
REGEXP_STANDARD = r"""
^
(?P<lnk>
    (?P<pth> .*[a-z0-9] )
    \:
    (?P<ln> \d+)
)
\:\s"""

# python '  File "test.py", line 13'
REGEXP_PYTHON = r"""
^\s\sFile\s
(?P<lnk>
    \"
    (?P<pth> [^\"]+ )
    \",\sline\s
    (?P<ln> \d+ )
),"""

# python 'test.sh: line 5:'
REGEXP_BASH = r"""
^(?P<lnk>
    (?P<pth> .* )
    \:\sline\s
    (?P<ln> \d+ )
)\:"""

# valac 'Test.vala:13.1-13.3: ...'
REGEXP_VALAC = r"""
^(?P<lnk>
    (?P<pth>
        .*vala
    )
    \:
    (?P<ln>
        \d+
    )
    \.\d+-\d+\.\d+
 )\: """

#ruby
#test.rb:5: ...
#	from test.rb:3:in `each'
# fist line parsed by REGEXP_STANDARD
REGEXP_RUBY = r"""
^\s+from\s
(?P<lnk>
    (?P<pth>
        .*
    )
    \:
    (?P<ln>
        \d+
    )
 )"""

# perl 'syntax error at test.pl line 88, near "$fake_var'
REGEXP_PERL = r"""
\sat\s
(?P<lnk>
    (?P<pth> .* )
    \sline\s
    (?P<ln> \d+ )
)"""

# mcs (C#) 'Test.cs(12,7): error CS0103: The name `fakeMethod'
REGEXP_MCS = r"""
^
(?P<lnk>
    (?P<pth> .*\.[cC][sS] )
    \(
    (?P<ln> \d+ )
    ,\d+\)
)
\:\s
"""

# ex:ts=4:et:
