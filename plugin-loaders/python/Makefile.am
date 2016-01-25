# C plugin loader

SUBDIRS = bindings
loaderdir = $(libdir)/xedit/plugin-loaders

AM_CPPFLAGS = \
	-I$(top_srcdir) 						\
	-I$(top_builddir)						\
	-I$(top_srcdir)/xedit						\
	-I$(top_builddir)/xedit						\
	$(XEDIT_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	$(PYGTK_CFLAGS)							\
	$(PYTHON_CFLAGS)						\
	-DXEDIT_PLUGINS_LIBS_DIR=\"$(XEDIT_PLUGINS_LIBS_DIR)\"		\
	-DXEDIT_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libpythonloader.la


NOINST_H_FILES = \
	xedit-plugin-loader-python.h	\
	xedit-plugin-python.h

libpythonloader_la_SOURCES = \
	xedit-plugin-loader-python.c 	\
	xedit-plugin-python.c		\
	$(NOINST_H_FILES)

libpythonloader_la_LDFLAGS = $(LOADER_LIBTOOL_FLAGS)
libpythonloader_la_LIBADD = \
	$(XEDIT_LIBS) 			\
	bindings/xedit.la

-include $(top_srcdir)/git.mk
