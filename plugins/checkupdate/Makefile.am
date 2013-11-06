# pluma win32 updater

plugindir = $(PLUMA_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = \
	-I$(top_srcdir) 						\
	$(PLUMA_CFLAGS) 						\
	$(LIBSOUP_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)

plugin_LTLIBRARIES = libcheckupdate.la

libcheckupdate_la_SOURCES = \
	pluma-check-update-plugin.h	\
	pluma-check-update-plugin.c

libcheckupdate_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libcheckupdate_la_LIBADD = $(PLUMA_LIBS) $(LIBSOUP_LIBS)

plugin_in_files = checkupdate.pluma-plugin.desktop.in

%.pluma-plugin: %.pluma-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.pluma-plugin.desktop.in=.pluma-plugin)

@INTLTOOL_XML_NOMERGE_RULE@
gsettings_SCHEMAS = org.mate.pluma.plugins.checkupdate.gschema.xml
@GSETTINGS_RULES@

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

EXTRA_DIST = $(plugin_in_files) $(gsettings_SCHEMAS).in.in

CLEANFILES = $(plugin_DATA) $(gsettings_SCHEMAS).in $(gsettings_SCHEMAS)

DISTCLEANFILES = $(plugin_DATA) $(gsettings_SCHEMAS).in $(gsettings_SCHEMAS)

-include $(top_srcdir)/git.mk
