# C plugin loader

SUBDIRS = bindings
loaderdir = $(libdir)/xed/plugin-loaders

AM_CPPFLAGS = \
	-I$(top_srcdir) 						\
	-I$(top_builddir)						\
	-I$(top_srcdir)/xed						\
	-I$(top_builddir)/xed						\
	$(XED_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	$(PYGTK_CFLAGS)							\
	$(PYTHON_CFLAGS)						\
	-DXED_PLUGINS_LIBS_DIR=\"$(XED_PLUGINS_LIBS_DIR)\"		\
	-DXED_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libpythonloader.la


NOINST_H_FILES = \
	xed-plugin-loader-python.h	\
	xed-plugin-python.h

libpythonloader_la_SOURCES = \
	xed-plugin-loader-python.c 	\
	xed-plugin-python.c		\
	$(NOINST_H_FILES)

libpythonloader_la_LDFLAGS = $(LOADER_LIBTOOL_FLAGS)
libpythonloader_la_LIBADD = \
	$(XED_LIBS) 			\
	bindings/xed.la

-include $(top_srcdir)/git.mk
