# gedit win32 updater

plugindir = $(GEDIT_PLUGINS_LIBS_DIR)

INCLUDES = \
	-I$(top_srcdir) 						\
	$(GEDIT_CFLAGS) 						\
	$(LIBSOUP_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)

plugin_LTLIBRARIES = libcheckupdate.la

libcheckupdate_la_SOURCES = \
	gedit-check-update-plugin.h	\
	gedit-check-update-plugin.c

libcheckupdate_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libcheckupdate_la_LIBADD = $(GEDIT_LIBS) $(LIBSOUP_LIBS)

plugin_in_files = checkupdate.gedit-plugin.desktop.in

%.gedit-plugin: %.gedit-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.gedit-plugin.desktop.in=.gedit-plugin)

schemasdir	 = $(MATECONF_SCHEMA_FILE_DIR)
schemas_in_files = gedit-check-update.schemas.in
schemas_DATA 	 = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

if MATECONF_SCHEMAS_INSTALL
install-data-local:
	        if test -z "$(DESTDIR)" ; then \
	                for p in $(schemas_DATA) ; do \
	                        MATECONF_CONFIG_SOURCE=$(MATECONF_SCHEMA_CONFIG_SOURCE) $(MATECONFTOOL) --makefile-install-rule $(top_builddir)/plugins/checkupdate/$$p ; \
	                done \
	        fi
else
install-data-local:
endif

EXTRA_DIST = $(plugin_in_files) $(schemas_in_files)

CLEANFILES = $(plugin_DATA) $(schemas_DATA)

DISTCLEANFILES = $(plugin_DATA) $(schemas_DATA)

-include $(top_srcdir)/git.mk
