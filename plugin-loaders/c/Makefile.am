# C plugin loader

loaderdir = $(libdir)/xedit/plugin-loaders

AM_CPPFLAGS = \
	-I$(top_srcdir) 						\
	$(XEDIT_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DXEDIT_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libcloader.la

NOINST_H_FILES = \
	xedit-plugin-loader-c.h

libcloader_la_SOURCES = \
	xedit-plugin-loader-c.c 	\
	$(NOINST_H_FILES)

libcloader_la_LDFLAGS = $(LOADER_LIBTOOL_FLAGS)
libcloader_la_LIBADD = $(XEDIT_LIBS)

-include $(top_srcdir)/git.mk
