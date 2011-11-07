DIST_SUBDIRS =		\
	changecase 	\
	checkupdate	\
	docinfo 	\
	externaltools	\
	filebrowser 	\
	modelines	\
	pythonconsole	\
	quickopen	\
	snippets	\
	sort 		\
	spell 		\
	taglist 	\
	time

SUBDIRS = 		\
	changecase	\
	docinfo		\
	filebrowser	\
	modelines	\
	sort		\
	taglist		\
	time

if ENABLE_PYTHON
SUBDIRS      += pythonconsole snippets quickopen

if !OS_WIN32
SUBDIRS      += externaltools
endif

endif

if ENABLE_ENCHANT
SUBDIRS      += spell
endif

if ENABLE_UPDATER
SUBDIRS      += checkupdate
endif

-include $(top_srcdir)/git.mk
