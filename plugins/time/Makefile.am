# time plugin
plugindir = $(PLUMA_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = \
	-I$(top_srcdir) 				\
	$(PLUMA_CFLAGS) 				\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)			

plugin_LTLIBRARIES = libtime.la

libtime_la_SOURCES = \
	pluma-time-plugin.h	\
	pluma-time-plugin.c

libtime_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libtime_la_LIBADD  = $(PLUMA_LIBS)

uidir = $(PLUMA_PLUGINS_DATA_DIR)/time
ui_DATA = \
	pluma-time-dialog.ui				\
	pluma-time-setup-dialog.ui

plugin_in_files = time.pluma-plugin.desktop.in

%.pluma-plugin: %.pluma-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.pluma-plugin.desktop.in=.pluma-plugin)

@INTLTOOL_XML_NOMERGE_RULE@
time_gschema_in = org.mate.pluma.plugins.time.gschema.xml.in
gsettings_SCHEMAS = $(time_gschema_in:.xml.in=.xml)
@GSETTINGS_RULES@

EXTRA_DIST = $(ui_DATA) $(plugin_in_files) $(time_gschema_in)

CLEANFILES = $(plugin_DATA) $(gsettings_SCHEMAS_in) $(gsettings_SCHEMASn)
DISTCLEANFILES = $(plugin_DATA) $(gsettings_SCHEMAS_in) $(gsettings_SCHEMAS)


-include $(top_srcdir)/git.mk
