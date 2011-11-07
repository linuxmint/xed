#!/usr/bin/perl

# gedit-tool-merge.pl
# This file is part of gedit
#
# Copyright (C) 2006 - Steve Frécinaux <code@istique.net>
#
# gedit is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# gedit is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with gedit; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, 
# Boston, MA  02110-1301  USA

# This script merges a script file with a desktop file containing
# metadata about the external tool. This is required in order to
# have translatable tools (bug #342042) since intltool can't extract
# string directly from tool files (a tool file being the combination
# of a script file and a metadata section).
#
# The desktop file is embedded in a comment of the script file, under
# the assumption that any scripting language supports # as a comment
# mark (this is likely to be true since the shebang uses #!). The
# section is placed at the top of the tool file, after the shebang and
# modelines if present.

use strict;
use warnings;
use Getopt::Long;

sub usage {
	print <<EOF;
Usage: ${0} [OPTION]... [SCRIPT] [DESKTOP]
Merges a script file with a desktop file, embedding it in a comment.

    -o, --output=FILE Specify the output file name. [default: stdout]
EOF
	exit;
}

my $output = "";
my $help;

GetOptions ("help|h" => \$help, "output|o=s" => \$output) or &usage;
usage if $help or @ARGV lt 2;

open INFILE, "<", $ARGV[0];
open DFILE, "<", $ARGV[1];
open STDOUT, ">", $output if $output;

# Put shebang and various modelines at the top of the generated file.
$_ = <INFILE>;
print and $_ = <INFILE> if /^#!/;
print and $_ = <INFILE> if /-\*-/;
print and $_ = <INFILE> if /(ex|vi|vim):/;

# Put a blank line before the info block if there is one in INFILE.
print and $_ = <INFILE> if /^\s*$/;
seek INFILE, -length, 1;

# Embed the desktop file...
print "# $_" while <DFILE>;
print "\n";

# ...and write the remaining part of the script.
print while <INFILE>;

close INFILE;
close DFILE;
close STDOUT;
