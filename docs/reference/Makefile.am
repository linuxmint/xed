## Process this file with automake to produce Makefile.in

# see /gtk-doc/examples/Makefile.am

# We require automake 1.6 at least.
AUTOMAKE_OPTIONS = 1.6

# The name of the module, e.g. 'glib'.
DOC_MODULE = xed

# The top-level XML file (SGML in the past).
DOC_MAIN_SGML_FILE = xed-docs.xml

# The directory containing the source code. Relative to $(srcdir).
DOC_SOURCE_DIR = ../../xed

# Extra options to supply to gtkdoc-scan.
# e.g. SCAN_OPTIONS = --deprecated-guards="GTK_DISABLE_DEPRECATED"
SCAN_OPTIONS = --rebuild-types

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS = --xml-mode --output-format=xml

# Used for dependencies. The docs will be rebuilt if any of these change.
HFILE_GLOB = $(top_srcdir)/xed/*.h
CFILE_GLOB = $(top_srcdir)/xed/*.c

# Header files to ignore when scanning (These are internal to xed).
IGNORE_HFILES=		\
	xed-close-confirmation-dialog.h		\
	xed-dirs.h 							\
	xed-commands.h						\
	xed-documents-panel.h			\
	xed-io-error-info-bar.h		\
	xed-languages-manager.h		\
	xed-plugins-engine.h			\
	xed-session.h				\
	xed-ui.h				\
	xed-window-private.h			\
	xed-output-window.h			\
	xed-encodings-dialog.h		\
	xed-open-location-dialog.h		\
	xed-page-setup-dialog.h		\
	xed-preferences-dialog.h		\
	xed-search-dialog.h			\
	xed-marshal.h				\
	xed-enum-types.h

# Images to copy into HTML directory.
HTML_IMAGES =

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
content_files =

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
GTKDOC_CFLAGS =				\
	-I$(top_srcdir)/xed		\
	-I$(top_builddir)/xed 	\
	-I$(top_srcdir)			\
	-I$(top_builddir)		\
	$(XED_CFLAGS)

GTKDOC_LIBS =						\
	$(top_builddir)/xed/libxed.la	\
	$(XED_LIBS)

MAINTAINERCLEANFILES = xed.types

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

-include $(top_srcdir)/git.mk
