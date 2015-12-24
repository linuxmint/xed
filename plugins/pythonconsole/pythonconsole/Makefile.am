# Python console plugin

plugindir = $(PLUMA_PLUGINS_LIBS_DIR)/pythonconsole
plugin_PYTHON =		\
	__init__.py	\
	console.py	\
	config.py

uidir = $(PLUMA_PLUGINS_DATA_DIR)/pythonconsole/ui
ui_DATA = config.ui

EXTRA_DIST = $(ui_DATA)

CLEANFILES =
DISTCLEANFILES =

-include $(top_srcdir)/git.mk
