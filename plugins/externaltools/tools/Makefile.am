# Python snippets plugin

plugindir = $(GEDIT_PLUGINS_LIBS_DIR)/externaltools
plugin_PYTHON =		\
	__init__.py	\
	capture.py	\
	library.py	\
	functions.py	\
	manager.py	\
	outputpanel.py	\
	filelookup.py	\
	linkparsing.py

uidir = $(GEDIT_PLUGINS_DATA_DIR)/externaltools/ui
ui_DATA = tools.ui \
	outputpanel.ui

EXTRA_DIST = $(ui_DATA)

CLEANFILES = *.bak *.gladep
DISTCLEANFILES = *.bak *.gladep

-include $(top_srcdir)/git.mk
