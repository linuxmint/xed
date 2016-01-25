# trailsave plugin
plugindir = $(XEDIT_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = \
	-I$(top_srcdir) 				\
	$(XEDIT_CFLAGS) 				\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)

plugin_LTLIBRARIES = libtrailsave.la

libtrailsave_la_SOURCES = \
	xedit-trail-save-plugin.h	\
	xedit-trail-save-plugin.c

libtrailsave_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libtrailsave_la_LIBADD  = $(XEDIT_LIBS)

plugin_in_files = trailsave.xedit-plugin.desktop.in

%.xedit-plugin: %.xedit-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.xedit-plugin.desktop.in=.xedit-plugin)

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)


-include $(top_srcdir)/git.mk
