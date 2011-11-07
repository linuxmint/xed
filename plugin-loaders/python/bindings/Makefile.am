## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = \
	pluma.la

nodist_pluma_la_SOURCES = 	\
	pluma.c			\
	plumautils.c		\
	plumacommands.c

pluma_la_LDFLAGS = \
	-module -avoid-version

pluma_la_LIBADD = \
	$(PYTHON_LIB_LOC)       \
	$(PYTHON_LIBS)		\
	$(PYTHON_EXTRA_LIBS)	\
	$(PYGTK_LIBS)

pluma_la_CFLAGS = \
	-I$(top_srcdir)					\
	-I$(top_builddir)				\
	-I$(top_srcdir)/pluma				\
	-I$(top_builddir)/pluma				\
	-I$(top_srcdir)/plugin-loaders/python		\
	-I$(top_builddir)/plugin-loaders/python		\
	$(PLUMA_CFLAGS)					\
	$(NO_STRICT_ALIASING_CFLAGS)			\
	$(PYGTK_CFLAGS)					\
	$(PYTHON_CFLAGS)				\
	$(AM_CFLAGS)

$(top_builddir)/pluma/pluma-enum-types.h:
	cd $(top_builddir)/pluma && $(MAKE) pluma-enum-types.h

pluma.c: pluma.defs pluma.override plumaplugin.override plumamessage.override $(top_builddir)/pluma/pluma-enum-types.h
	( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--register $(PYGTK_DEFSDIR)/pango-types.defs \
		--register $(PYGTK_DEFSDIR)/gdk-types.defs \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--register $(PYGTK_DEFSDIR)/gtksourceview2.defs \
		--register $(PYGOBJECT_DEFSDIR)/gio-types.defs \
		--override $*.override \
		--prefix py$* $(<F) ) > $@ 

plumautils.c: plumautils.defs plumautils.override
	( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--override $*.override \
		--prefix py$* $(<F) ) > $@ 

plumacommands.c: plumacommands.defs plumacommands.override pluma.defs
	( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--register pluma.defs \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--override $*.override \
		--prefix py$* $(<F) ) > $@ 

BINDING_PLUMA_HEADERS_SRCDIR_IN = \
	pluma/pluma-app.h		\
	pluma/pluma-document.h		\
	pluma/pluma-encodings.h		\
	pluma/pluma-plugin.h		\
	plugin-loaders/python/pluma-plugin-python.h \
	pluma/pluma-view.h		\
	pluma/pluma-statusbar.h		\
	pluma/pluma-tab.h 		\
	pluma/pluma-panel.h 		\
	pluma/pluma-window.h 		\
	pluma/pluma-help.h		\
	pluma/pluma-debug.h		\
	pluma/pluma-message-type.h	\
	pluma/pluma-message.h		\
	pluma/pluma-message-bus.h	\
	pluma/pluma-language-manager.h

BINDING_UTILS_HEADERS_SRCDIR_IN = \
	pluma/pluma-utils.h

BINDING_COMMANDS_HEADERS_SRCDIR_IN = \
	pluma/pluma-commands.h

BINDING_HEADERS_BUILDDIR_IN = 

BINDING_PLUMA_HEADERS_SRCDIR	:= $(addprefix $(top_srcdir)/,$(BINDING_PLUMA_HEADERS_SRCDIR_IN))
BINDING_UTILS_HEADERS_SRCDIR	:= $(addprefix $(top_srcdir)/,$(BINDING_UTILS_HEADERS_SRCDIR_IN))
BINDING_COMMANDS_HEADERS_SRCDIR	:= $(addprefix $(top_srcdir)/,$(BINDING_COMMANDS_HEADERS_SRCDIR_IN))

BINDING_HEADERS_BUILDDIR	:= $(addprefix $(top_builddir)/,$(BINDING_HEADERS_BUILDDIR_IN))

regenerate-python-binding:
	$(PYGTK_H2DEF) $(sort $(BINDING_PLUMA_HEADERS_SRCDIR) $(BINDING_HEADERS_BUILDDIR)) > pluma.defs.new
	$(PYGTK_H2DEF) $(sort $(BINDING_UTILS_HEADERS_SRCDIR) $(BINDING_HEADERS_BUILDDIR)) > plumautils.defs.new
	$(PYGTK_H2DEF) $(sort $(BINDING_COMMANDS_HEADERS_SRCDIR) $(BINDING_HEADERS_BUILDDIR)) > plumacommands.defs.new

BUILT_SOURCES = \
	pluma.c		\
	plumautils.c	\
	plumacommands.c

EXTRA_DIST = \
	pluma.override		\
	pluma.defs		\
	plumautils.override	\
	plumautils.defs		\
	plumacommands.override 	\
	plumacommands.defs	\
	plumamessage.override	\
	plumaplugin.override

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
