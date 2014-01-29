# ##(PLUGIN_NAME)

plugindir = $(PLUMA_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = \
	-I$(top_srcdir) 						\
	$(PLUMA_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DPLUMA_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

plugin_LTLIBRARIES = lib##(PLUGIN_MODULE).la

lib##(PLUGIN_MODULE)_la_SOURCES = \
	##(PLUGIN_MODULE)-plugin.h	\
	##(PLUGIN_MODULE)-plugin.c

lib##(PLUGIN_MODULE)_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
lib##(PLUGIN_MODULE)_la_LIBADD  = $(PLUMA_LIBS)

# UI files (if you use gtkbuilder for your plugin, list those files here)
uidir = $(PLUMA_PLUGINS_DATA_DIR)/##(PLUGIN_MODULE)
ui_DATA =

plugin_in_files = ##(PLUGIN_MODULE).pluma-plugin.desktop.in

%.pluma-plugin: %.pluma-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.pluma-plugin.desktop.in=.pluma-plugin)

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(plugin_DATA) $(ui_DATA)

DISTCLEANFILES = $(plugin_DATA) $(ui_DATA)

-include $(top_srcdir)/git.mk
