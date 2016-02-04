# C plugin loader

loaderdir = $(libdir)/xed/plugin-loaders

AM_CPPFLAGS = \
	-I$(top_srcdir) 						\
	$(XED_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DXED_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libcloader.la

NOINST_H_FILES = \
	xed-plugin-loader-c.h

libcloader_la_SOURCES = \
	xed-plugin-loader-c.c 	\
	$(NOINST_H_FILES)

libcloader_la_LDFLAGS = $(LOADER_LIBTOOL_FLAGS)
libcloader_la_LIBADD = $(XED_LIBS)

-include $(top_srcdir)/git.mk
