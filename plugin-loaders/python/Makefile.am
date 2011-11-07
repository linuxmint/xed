# C plugin loader

SUBDIRS = bindings
loaderdir = $(libdir)/gedit-2/plugin-loaders

INCLUDES = \
	-I$(top_srcdir) 						\
	-I$(top_builddir)						\
	-I$(top_srcdir)/gedit						\
	-I$(top_builddir)/gedit						\
	$(GEDIT_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	$(PYGTK_CFLAGS)							\
	$(PYTHON_CFLAGS)						\
	-DGEDIT_PLUGINS_LIBS_DIR=\"$(GEDIT_PLUGINS_LIBS_DIR)\"		\
	-DGEDIT_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libpythonloader.la


NOINST_H_FILES = \
	gedit-plugin-loader-python.h	\
	gedit-plugin-python.h

libpythonloader_la_SOURCES = \
	gedit-plugin-loader-python.c 	\
	gedit-plugin-python.c		\
	$(NOINST_H_FILES)

libpythonloader_la_LDFLAGS = $(LOADER_LIBTOOL_FLAGS)
libpythonloader_la_LIBADD = \
	$(GEDIT_LIBS) 			\
	bindings/gedit.la

-include $(top_srcdir)/git.mk
