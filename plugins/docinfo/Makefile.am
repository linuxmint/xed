# docinfo plugin
plugindir = $(PLUMA_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = \
	-I$(top_srcdir) 				\
	$(PLUMA_CFLAGS) 				\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)

plugin_LTLIBRARIES = libdocinfo.la

libdocinfo_la_SOURCES = \
	pluma-docinfo-plugin.h	\
	pluma-docinfo-plugin.c

libdocinfo_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libdocinfo_la_LIBADD  = $(PLUMA_LIBS)

uidir = $(PLUMA_PLUGINS_DATA_DIR)/docinfo
ui_DATA = docinfo.ui

plugin_in_files = docinfo.pluma-plugin.desktop.in

%.pluma-plugin: %.pluma-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.pluma-plugin.desktop.in=.pluma-plugin)

EXTRA_DIST = $(ui_DATA) $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)


-include $(top_srcdir)/git.mk
