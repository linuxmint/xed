## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = \
	xedit.la

nodist_xedit_la_SOURCES = 	\
	xedit.c			\
	xeditutils.c		\
	xeditcommands.c

xedit_la_LDFLAGS = \
	-module -avoid-version

xedit_la_LIBADD = \
	$(PYTHON_LIB_LOC)       \
	$(PYTHON_LIBS)		\
	$(PYTHON_EXTRA_LIBS)	\
	$(PYGTK_LIBS)

xedit_la_CFLAGS = \
	-I$(top_srcdir)					\
	-I$(top_builddir)				\
	-I$(top_srcdir)/xedit				\
	-I$(top_builddir)/xedit				\
	-I$(top_srcdir)/plugin-loaders/python		\
	-I$(top_builddir)/plugin-loaders/python		\
	$(XEDIT_CFLAGS)					\
	$(NO_STRICT_ALIASING_CFLAGS)			\
	$(PYGTK_CFLAGS)					\
	$(PYTHON_CFLAGS)				\
	$(AM_CFLAGS)

$(top_builddir)/xedit/xedit-enum-types.h:
	cd $(top_builddir)/xedit && $(MAKE) xedit-enum-types.h

xedit.c: xedit.defs xedit.override xeditplugin.override xeditmessage.override $(top_builddir)/xedit/xedit-enum-types.h
	( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--register $(PYGTK_DEFSDIR)/pango-types.defs \
		--register $(PYGTK_DEFSDIR)/gdk-types.defs \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--register $(PYGTK_DEFSDIR)/gtksourceview2.defs \
		--register $(PYGOBJECT_DEFSDIR)/gio-types.defs \
		--override $*.override \
		--prefix py$* $(<F) ) > $@ 

xeditutils.c: xeditutils.defs xeditutils.override
	( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--override $*.override \
		--prefix py$* $(<F) ) > $@ 

xeditcommands.c: xeditcommands.defs xeditcommands.override xedit.defs
	( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--register xedit.defs \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--override $*.override \
		--prefix py$* $(<F) ) > $@ 

BINDING_XEDIT_HEADERS_SRCDIR_IN = \
	xedit/xedit-app.h		\
	xedit/xedit-document.h		\
	xedit/xedit-encodings.h		\
	xedit/xedit-plugin.h		\
	plugin-loaders/python/xedit-plugin-python.h \
	xedit/xedit-view.h		\
	xedit/xedit-statusbar.h		\
	xedit/xedit-tab.h 		\
	xedit/xedit-panel.h 		\
	xedit/xedit-window.h 		\
	xedit/xedit-help.h		\
	xedit/xedit-debug.h		\
	xedit/xedit-message-type.h	\
	xedit/xedit-message.h		\
	xedit/xedit-message-bus.h	\
	xedit/xedit-language-manager.h

BINDING_UTILS_HEADERS_SRCDIR_IN = \
	xedit/xedit-utils.h

BINDING_COMMANDS_HEADERS_SRCDIR_IN = \
	xedit/xedit-commands.h

BINDING_HEADERS_BUILDDIR_IN = 

BINDING_XEDIT_HEADERS_SRCDIR	:= $(addprefix $(top_srcdir)/,$(BINDING_XEDIT_HEADERS_SRCDIR_IN))
BINDING_UTILS_HEADERS_SRCDIR	:= $(addprefix $(top_srcdir)/,$(BINDING_UTILS_HEADERS_SRCDIR_IN))
BINDING_COMMANDS_HEADERS_SRCDIR	:= $(addprefix $(top_srcdir)/,$(BINDING_COMMANDS_HEADERS_SRCDIR_IN))

BINDING_HEADERS_BUILDDIR	:= $(addprefix $(top_builddir)/,$(BINDING_HEADERS_BUILDDIR_IN))

regenerate-python-binding:
	$(PYGTK_H2DEF) $(sort $(BINDING_XEDIT_HEADERS_SRCDIR) $(BINDING_HEADERS_BUILDDIR)) > xedit.defs.new
	$(PYGTK_H2DEF) $(sort $(BINDING_UTILS_HEADERS_SRCDIR) $(BINDING_HEADERS_BUILDDIR)) > xeditutils.defs.new
	$(PYGTK_H2DEF) $(sort $(BINDING_COMMANDS_HEADERS_SRCDIR) $(BINDING_HEADERS_BUILDDIR)) > xeditcommands.defs.new

BUILT_SOURCES = \
	xedit.c		\
	xeditutils.c	\
	xeditcommands.c

EXTRA_DIST = \
	xedit.override		\
	xedit.defs		\
	xeditutils.override	\
	xeditutils.defs		\
	xeditcommands.override 	\
	xeditcommands.defs	\
	xeditmessage.override	\
	xeditplugin.override

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
