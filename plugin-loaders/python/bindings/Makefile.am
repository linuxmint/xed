## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = \
	xed.la

nodist_xed_la_SOURCES = 	\
	xed.c			\
	xedutils.c		\
	xedcommands.c

xed_la_LDFLAGS = \
	-module -avoid-version

xed_la_LIBADD = \
	$(PYTHON_LIB_LOC)       \
	$(PYTHON_LIBS)		\
	$(PYTHON_EXTRA_LIBS)	\
	$(PYGTK_LIBS)

xed_la_CFLAGS = \
	-I$(top_srcdir)					\
	-I$(top_builddir)				\
	-I$(top_srcdir)/xed				\
	-I$(top_builddir)/xed				\
	-I$(top_srcdir)/plugin-loaders/python		\
	-I$(top_builddir)/plugin-loaders/python		\
	$(XED_CFLAGS)					\
	$(NO_STRICT_ALIASING_CFLAGS)			\
	$(PYGTK_CFLAGS)					\
	$(PYTHON_CFLAGS)				\
	$(AM_CFLAGS)

$(top_builddir)/xed/xed-enum-types.h:
	cd $(top_builddir)/xed && $(MAKE) xed-enum-types.h

xed.c: xed.defs xed.override xedplugin.override xedmessage.override $(top_builddir)/xed/xed-enum-types.h
	( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--register $(PYGTK_DEFSDIR)/pango-types.defs \
		--register $(PYGTK_DEFSDIR)/gdk-types.defs \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--register $(PYGTK_DEFSDIR)/gtksourceview2.defs \
		--register $(PYGOBJECT_DEFSDIR)/gio-types.defs \
		--override $*.override \
		--prefix py$* $(<F) ) > $@ 

xedutils.c: xedutils.defs xedutils.override
	( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--override $*.override \
		--prefix py$* $(<F) ) > $@ 

xedcommands.c: xedcommands.defs xedcommands.override xed.defs
	( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--register xed.defs \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--override $*.override \
		--prefix py$* $(<F) ) > $@ 

BINDING_XED_HEADERS_SRCDIR_IN = \
	xed/xed-app.h		\
	xed/xed-document.h		\
	xed/xed-encodings.h		\
	xed/xed-plugin.h		\
	plugin-loaders/python/xed-plugin-python.h \
	xed/xed-view.h		\
	xed/xed-statusbar.h		\
	xed/xed-tab.h 		\
	xed/xed-panel.h 		\
	xed/xed-window.h 		\
	xed/xed-help.h		\
	xed/xed-debug.h		\
	xed/xed-message-type.h	\
	xed/xed-message.h		\
	xed/xed-message-bus.h	\
	xed/xed-language-manager.h

BINDING_UTILS_HEADERS_SRCDIR_IN = \
	xed/xed-utils.h

BINDING_COMMANDS_HEADERS_SRCDIR_IN = \
	xed/xed-commands.h

BINDING_HEADERS_BUILDDIR_IN = 

BINDING_XED_HEADERS_SRCDIR	:= $(addprefix $(top_srcdir)/,$(BINDING_XED_HEADERS_SRCDIR_IN))
BINDING_UTILS_HEADERS_SRCDIR	:= $(addprefix $(top_srcdir)/,$(BINDING_UTILS_HEADERS_SRCDIR_IN))
BINDING_COMMANDS_HEADERS_SRCDIR	:= $(addprefix $(top_srcdir)/,$(BINDING_COMMANDS_HEADERS_SRCDIR_IN))

BINDING_HEADERS_BUILDDIR	:= $(addprefix $(top_builddir)/,$(BINDING_HEADERS_BUILDDIR_IN))

regenerate-python-binding:
	$(PYGTK_H2DEF) $(sort $(BINDING_XED_HEADERS_SRCDIR) $(BINDING_HEADERS_BUILDDIR)) > xed.defs.new
	$(PYGTK_H2DEF) $(sort $(BINDING_UTILS_HEADERS_SRCDIR) $(BINDING_HEADERS_BUILDDIR)) > xedutils.defs.new
	$(PYGTK_H2DEF) $(sort $(BINDING_COMMANDS_HEADERS_SRCDIR) $(BINDING_HEADERS_BUILDDIR)) > xedcommands.defs.new

BUILT_SOURCES = \
	xed.c		\
	xedutils.c	\
	xedcommands.c

EXTRA_DIST = \
	xed.override		\
	xed.defs		\
	xedutils.override	\
	xedutils.defs		\
	xedcommands.override 	\
	xedcommands.defs	\
	xedmessage.override	\
	xedplugin.override

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
