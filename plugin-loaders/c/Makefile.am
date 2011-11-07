# C plugin loader

loaderdir = $(libdir)/gedit-2/plugin-loaders

INCLUDES = \
	-I$(top_srcdir) 						\
	$(GEDIT_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DGEDIT_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libcloader.la

NOINST_H_FILES = \
	gedit-plugin-loader-c.h

libcloader_la_SOURCES = \
	gedit-plugin-loader-c.c 	\
	$(NOINST_H_FILES)

libcloader_la_LDFLAGS = $(LOADER_LIBTOOL_FLAGS)
libcloader_la_LIBADD = $(GEDIT_LIBS)

-include $(top_srcdir)/git.mk
