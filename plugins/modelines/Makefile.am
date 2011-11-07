# Modelines Plugin
plugindir = $(GEDIT_PLUGINS_LIBS_DIR)

INCLUDES = \
	-I$(top_srcdir) 				\
	$(GEDIT_CFLAGS) 				\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)

modelinesdir = $(GEDIT_PLUGINS_DATA_DIR)/modelines
modelines_DATA = \
	language-mappings

plugin_LTLIBRARIES = libmodelines.la

libmodelines_la_SOURCES = \
	gedit-modeline-plugin.h				\
	gedit-modeline-plugin.c				\
	modeline-parser.h				\
	modeline-parser.c

libmodelines_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libmodelines_la_LIBADD  = $(GEDIT_LIBS)

plugin_in_files = modelines.gedit-plugin.desktop.in
%.gedit-plugin: %.gedit-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.gedit-plugin.desktop.in=.gedit-plugin)

EXTRA_DIST = \
	$(plugin_in_files)				\
	$(modelines_DATA)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)


-include $(top_srcdir)/git.mk
