# C plugin loader

SUBDIRS = bindings
loaderdir = $(libdir)/pluma/plugin-loaders

AM_CPPFLAGS = \
	-I$(top_srcdir) 						\
	-I$(top_builddir)						\
	-I$(top_srcdir)/pluma						\
	-I$(top_builddir)/pluma						\
	$(PLUMA_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	$(PYGTK_CFLAGS)							\
	$(PYTHON_CFLAGS)						\
	-DPLUMA_PLUGINS_LIBS_DIR=\"$(PLUMA_PLUGINS_LIBS_DIR)\"		\
	-DPLUMA_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libpythonloader.la


NOINST_H_FILES = \
	pluma-plugin-loader-python.h	\
	pluma-plugin-python.h

libpythonloader_la_SOURCES = \
	pluma-plugin-loader-python.c 	\
	pluma-plugin-python.c		\
	$(NOINST_H_FILES)

libpythonloader_la_LDFLAGS = $(LOADER_LIBTOOL_FLAGS)
libpythonloader_la_LIBADD = \
	$(PLUMA_LIBS) 			\
	bindings/pluma.la

-include $(top_srcdir)/git.mk
