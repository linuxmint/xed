# time plugin
plugindir = $(XEDIT_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = \
	-I$(top_srcdir) 				\
	$(XEDIT_CFLAGS) 				\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)			

plugin_LTLIBRARIES = libtime.la

libtime_la_SOURCES = \
	xedit-time-plugin.h	\
	xedit-time-plugin.c

libtime_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libtime_la_LIBADD  = $(XEDIT_LIBS)

uidir = $(XEDIT_PLUGINS_DATA_DIR)/time
ui_DATA = \
	xedit-time-dialog.ui				\
	xedit-time-setup-dialog.ui

plugin_in_files = time.xedit-plugin.desktop.in

%.xedit-plugin: %.xedit-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.xedit-plugin.desktop.in=.xedit-plugin)

@INTLTOOL_XML_NOMERGE_RULE@
time_gschema_in = org.x.editor.plugins.time.gschema.xml.in
gsettings_SCHEMAS = $(time_gschema_in:.xml.in=.xml)
@GSETTINGS_RULES@

EXTRA_DIST = $(ui_DATA) $(plugin_in_files) $(time_gschema_in)

CLEANFILES = $(plugin_DATA) $(gsettings_SCHEMAS_in) $(gsettings_SCHEMASn)
DISTCLEANFILES = $(plugin_DATA) $(gsettings_SCHEMAS_in) $(gsettings_SCHEMAS)


-include $(top_srcdir)/git.mk
