# Python snippets plugin
plugindir = $(GEDIT_PLUGINS_LIBS_DIR)/snippets

plugin_PYTHON = \
	__init__.py \
	WindowHelper.py \
	Document.py \
	Library.py \
	Snippet.py \
	Parser.py \
	Placeholder.py \
	Manager.py \
	Helper.py \
	SubstitutionParser.py \
	Importer.py \
	Exporter.py \
	LanguageManager.py \
	Completion.py

uidir = $(GEDIT_PLUGINS_DATA_DIR)/snippets/ui
ui_DATA = snippets.ui

EXTRA_DIST = $(ui_DATA)

CLEANFILES = *.bak *.gladep *.pyc
DISTCLEANFILES = *.bak *.gladep *.pyc

-include $(top_srcdir)/git.mk
