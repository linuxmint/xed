# C plugin loader

loaderdir = $(libdir)/pluma/plugin-loaders

AM_CPPFLAGS = \
	-I$(top_srcdir) 						\
	$(PLUMA_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DPLUMA_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libcloader.la

NOINST_H_FILES = \
	pluma-plugin-loader-c.h

libcloader_la_SOURCES = \
	pluma-plugin-loader-c.c 	\
	$(NOINST_H_FILES)

libcloader_la_LDFLAGS = $(LOADER_LIBTOOL_FLAGS)
libcloader_la_LIBADD = $(PLUMA_LIBS)

-include $(top_srcdir)/git.mk
