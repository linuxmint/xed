INCLUDES = -g -I$(top_srcdir) -I$(top_srcdir)/gedit $(GEDIT_DEBUG_FLAGS) $(GEDIT_CFLAGS)

noinst_PROGRAMS = $(TEST_PROGS)
progs_ldadd     = $(top_builddir)/gedit/libgedit.la

TEST_PROGS			= smart-converter
smart_converter_SOURCES		= smart-converter.c
smart_converter_LDADD		= $(progs_ldadd)

TEST_PROGS			+= document-input-stream
document_input_stream_SOURCES	= document-input-stream.c
document_input_stream_LDADD	= $(progs_ldadd)

TEST_PROGS			+= document-output-stream
document_output_stream_SOURCES	= document-output-stream.c
document_output_stream_LDADD	= $(progs_ldadd)

TEST_PROGS			+= document-loader
document_loader_SOURCES		= document-loader.c
document_loader_LDADD		= $(progs_ldadd)

TEST_PROGS			+= document-saver
document_saver_SOURCES		= document-saver.c
document_saver_LDADD		= $(progs_ldadd)

TESTS = $(TEST_PROGS)

EXTRA_DIST = setup-document-saver.sh
