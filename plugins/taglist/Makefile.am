# Tag list plugin
plugindir = $(PLUMA_PLUGINS_LIBS_DIR)

taglistdir = $(PLUMA_PLUGINS_DATA_DIR)/taglist

taglist_in_files = \
	HTML.tags.xml.in	\
	Latex.tags.xml.in	\
	XSLT.tags.xml.in \
	XUL.tags.xml.in

taglist_DATA = $(taglist_in_files:.tags.xml.in=.tags.gz)

AM_CPPFLAGS = 							\
	-I$(top_srcdir) 					\
	$(PLUMA_CFLAGS) 					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED_CFLAGS)	

plugin_LTLIBRARIES = libtaglist.la

libtaglist_la_SOURCES = \
	pluma-taglist-plugin-parser.c	\
	pluma-taglist-plugin-parser.h	\
	pluma-taglist-plugin-panel.c	\
	pluma-taglist-plugin-panel.h	\
	pluma-taglist-plugin.c		\
	pluma-taglist-plugin.h

libtaglist_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libtaglist_la_LIBADD  = $(PLUMA_LIBS)

plugin_in_files = taglist.pluma-plugin.desktop.in

%.pluma-plugin: %.pluma-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po)
	$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

GZIP_ENV = -9

%.tags.gz: %.tags.xml.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po)
	LC_ALL=C $(INTLTOOL_MERGE) $(top_srcdir)/po $< $(@:.gz=) -x -u -c $(top_builddir)/po/.intltool-merge-cache
	GZIP=$(GZIP_ENV) gzip -f $(@:.gz=)

plugin_DATA = $(plugin_in_files:.pluma-plugin.desktop.in=.pluma-plugin)

EXTRA_DIST = \
	$(taglist_in_files) $(taglist_DATA)	\
	$(plugin_in_files) $(plugin_DATA)

CLEANFILES = \
	$(taglist_DATA)	\
	$(plugin_DATA)
DISTCLEANFILES = \
	$(taglist_DATA)	\
	$(plugin_DATA)




-include $(top_srcdir)/git.mk
