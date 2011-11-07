# time plugin
plugindir = $(GEDIT_PLUGINS_LIBS_DIR)

INCLUDES = \
	-I$(top_srcdir) 				\
	$(GEDIT_CFLAGS) 				\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)			

plugin_LTLIBRARIES = libtime.la

libtime_la_SOURCES = \
	gedit-time-plugin.h	\
	gedit-time-plugin.c

libtime_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libtime_la_LIBADD  = $(GEDIT_LIBS)

uidir = $(GEDIT_PLUGINS_DATA_DIR)/time
ui_DATA = \
	gedit-time-dialog.ui				\
	gedit-time-setup-dialog.ui

plugin_in_files = time.gedit-plugin.desktop.in

%.gedit-plugin: %.gedit-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.gedit-plugin.desktop.in=.gedit-plugin)

EXTRA_DIST = $(ui_DATA) $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)


-include $(top_srcdir)/git.mk
