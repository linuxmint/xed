DIST_SUBDIRS =		\
	changecase 	\
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
	time		\
	trailsave

SUBDIRS = 		\
	changecase	\
	docinfo		\
	externaltools \
	filebrowser	\
	modelines	\
	sort		\
	taglist		\
	time		\
	trailsave

if ENABLE_PYTHON
SUBDIRS      += pythonconsole snippets quickopen
endif

if ENABLE_ENCHANT
SUBDIRS      += spell
endif

-include $(top_srcdir)/git.mk
