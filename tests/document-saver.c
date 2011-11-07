/*
 * document-saver.c
 * This file is part of gedit
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
 *
 * gedit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * gedit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gedit; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#include "gedit-gio-document-loader.h"
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include <sys/stat.h>

#define DEFAULT_LOCAL_URI "/tmp/gedit-document-saver-test.txt"
#define DEFAULT_REMOTE_URI "sftp://localhost/tmp/gedit-document-saver-test.txt"
#define DEFAULT_CONTENT "hello world!"
#define DEFAULT_CONTENT_RESULT "hello world!\n"

#define UNOWNED_LOCAL_DIRECTORY "/tmp/gedit-document-saver-unowned"
#define UNOWNED_LOCAL_URI "/tmp/gedit-document-saver-unowned/gedit-document-saver-test.txt"

#define UNOWNED_REMOTE_DIRECTORY "sftp://localhost/tmp/gedit-document-saver-unowned"
#define UNOWNED_REMOTE_URI "sftp://localhost/tmp/gedit-document-saver-unowned/gedit-document-saver-test.txt"

#define UNOWNED_GROUP_LOCAL_URI "/tmp/gedit-document-saver-unowned-group.txt"
#define UNOWNED_GROUP_REMOTE_URI "sftp://localhost/tmp/gedit-document-saver-unowned-group.txt"

static gboolean test_completed;
static gboolean mount_completed;
static gboolean mount_success;

typedef struct
{
	gchar *uri;
	const gchar *test_contents;
	gpointer data;
} SaverTestData;

static SaverTestData *
saver_test_data_new (const gchar *uri, const gchar *test_contents, gpointer data)
{
	SaverTestData *ret = g_slice_new (SaverTestData);

	ret->uri = g_strdup (uri);
	ret->test_contents = test_contents;
	ret->data = data;

	return ret;
}

static void
saver_test_data_free (SaverTestData *data)
{
	if (data == NULL)
	{
		return;
	}

	g_free (data->uri);
	g_slice_free (SaverTestData, data);
}

static GeditDocument *
create_document (const gchar *contents)
{
	GeditDocument *document = gedit_document_new ();

	gtk_text_buffer_set_text (GTK_TEXT_BUFFER (document), contents, -1);
	return document;
}

static void
complete_test_error (GeditDocument *document,
                     GError        *error,
                     SaverTestData *data)
{
	g_assert_no_error (error);
}

static const gchar *
read_file (const gchar *uri)
{
	GFile *file = g_file_new_for_commandline_arg (uri);
	GError *error = NULL;
	static gchar buffer[4096];
	gsize read;

	GInputStream *stream = G_INPUT_STREAM (g_file_read (file, NULL, &error));

	g_assert_no_error (error);

	g_input_stream_read_all (stream, buffer, sizeof (buffer) - 1, &read, NULL, &error);
	g_assert_no_error (error);

	buffer[read] = '\0';

	g_input_stream_close (stream, NULL, NULL);

	g_object_unref (stream);
	g_object_unref (file);

	return buffer;
}

static void
complete_test (GeditDocument *document,
               GError        *error,
               SaverTestData *data)
{
	test_completed = TRUE;

	if (data && data->test_contents && data->uri)
	{
		g_assert_cmpstr (data->test_contents, ==, read_file (data->uri));
	}
}

static void
mount_ready_callback (GObject      *object,
                      GAsyncResult *result,
                      gpointer      data)
{
	GError *error = NULL;
	mount_success = g_file_mount_enclosing_volume_finish (G_FILE (object),
	                                                      result,
	                                                      &error);

	if (error && error->code == G_IO_ERROR_ALREADY_MOUNTED)
	{
		mount_success = TRUE;
		g_error_free (error);
	}
	else
	{
		g_assert_no_error (error);
	}

	mount_completed = TRUE;
}

static gboolean
ensure_mounted (GFile *file)
{
	GMountOperation *mo;

	mount_success = FALSE;
	mount_completed = FALSE;

	if (g_file_is_native (file))
	{
		return TRUE;
	}

	mo = gtk_mount_operation_new (NULL);

	g_file_mount_enclosing_volume (file,
	                               G_MOUNT_MOUNT_NONE,
	                               mo,
	                               NULL,
	                               mount_ready_callback,
	                               NULL);

	while (!mount_completed)
	{
		g_main_context_iteration (NULL, TRUE);
	}

	g_object_unref (mo);

	return mount_success;
}

static void
test_saver (const gchar              *filename_or_uri,
            const gchar              *contents,
            GeditDocumentNewlineType  newline_type,
            GeditDocumentSaveFlags    save_flags,
            GCallback                 saved_callback,
            SaverTestData            *data)
{
	GFile *file;
	gchar *uri;
	GeditDocument *document;
	gboolean existed;

	document = create_document (contents);
	gedit_document_set_newline_type (document, newline_type);

	g_signal_connect (document, "saved", G_CALLBACK (complete_test_error), data);

	if (saved_callback)
	{
		g_signal_connect (document, "saved", saved_callback, data);
	}

	g_signal_connect_after (document, "saved", G_CALLBACK (complete_test), data);

	test_completed = FALSE;

	file = g_file_new_for_commandline_arg (filename_or_uri);
	uri = g_file_get_uri (file);
	existed = g_file_query_exists (file, NULL);

	ensure_mounted (file);

	gedit_document_save_as (document, uri, gedit_encoding_get_utf8 (), save_flags);

	while (!test_completed)
	{
		g_main_context_iteration (NULL, TRUE);
	}

	if (!existed)
	{
		g_file_delete (file, NULL, NULL);
	}

	g_free (uri);
	g_object_unref (file);

	saver_test_data_free (data);
}

typedef struct
{
	GeditDocumentNewlineType type;
	const gchar *text;
	const gchar *result;
} NewLineTestData;

static NewLineTestData newline_test_data[] = {
	{GEDIT_DOCUMENT_NEWLINE_TYPE_LF, "\nhello\nworld", "\nhello\nworld\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_LF, "\nhello\nworld\n", "\nhello\nworld\n\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_LF, "\nhello\nworld\n\n", "\nhello\nworld\n\n\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_LF, "\r\nhello\r\nworld", "\nhello\nworld\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_LF, "\r\nhello\r\nworld\r\n", "\nhello\nworld\n\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_LF, "\rhello\rworld", "\nhello\nworld\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_LF, "\rhello\rworld\r", "\nhello\nworld\n\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_LF, "\nhello\r\nworld", "\nhello\nworld\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_LF, "\nhello\r\nworld\r", "\nhello\nworld\n\n"},

	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF, "\nhello\nworld", "\r\nhello\r\nworld\r\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF, "\nhello\nworld\n", "\r\nhello\r\nworld\r\n\r\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF, "\nhello\nworld\n\n", "\r\nhello\r\nworld\r\n\r\n\r\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF, "\r\nhello\r\nworld", "\r\nhello\r\nworld\r\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF, "\r\nhello\r\nworld\r\n", "\r\nhello\r\nworld\r\n\r\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF, "\rhello\rworld", "\r\nhello\r\nworld\r\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF, "\rhello\rworld\r", "\r\nhello\r\nworld\r\n\r\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF, "\nhello\r\nworld", "\r\nhello\r\nworld\r\n"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF, "\nhello\r\nworld\r", "\r\nhello\r\nworld\r\n\r\n"},

	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR, "\nhello\nworld", "\rhello\rworld\r"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR, "\nhello\nworld\n", "\rhello\rworld\r\r"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR, "\nhello\nworld\n\n", "\rhello\rworld\r\r\r"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR, "\r\nhello\r\nworld", "\rhello\rworld\r"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR, "\r\nhello\r\nworld\r\n", "\rhello\rworld\r\r"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR, "\rhello\rworld", "\rhello\rworld\r"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR, "\rhello\rworld\r", "\rhello\rworld\r\r"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR, "\nhello\r\nworld", "\rhello\rworld\r"},
	{GEDIT_DOCUMENT_NEWLINE_TYPE_CR, "\nhello\r\nworld\r", "\rhello\rworld\r\r"}
};

static void
test_new_line (const gchar *filename, GeditDocumentSaveFlags save_flags)
{
	gint i;
	gint num = sizeof (newline_test_data) / sizeof (NewLineTestData);

	for (i = 0; i < num; ++i)
	{
		NewLineTestData *nt = &(newline_test_data[i]);

		test_saver (filename,
		            nt->text,
		            nt->type,
		            save_flags,
		            NULL,
		            saver_test_data_new (filename, nt->result, NULL));
	}
}

static void
test_local_newline ()
{
	test_new_line (DEFAULT_LOCAL_URI, 0);
}

static void
test_local ()
{
	test_saver (DEFAULT_LOCAL_URI,
	            "hello world",
	            GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	            0,
	            NULL,
	            saver_test_data_new (DEFAULT_LOCAL_URI, "hello world\n", NULL));

	test_saver (DEFAULT_LOCAL_URI,
	            "hello world\r\n",
	            GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	            0,
	            NULL,
	            saver_test_data_new (DEFAULT_LOCAL_URI, "hello world\n\n", NULL));

	test_saver (DEFAULT_LOCAL_URI,
	            "hello world\n",
	            GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	            0,
	            NULL,
	            saver_test_data_new (DEFAULT_LOCAL_URI, "hello world\n\n", NULL));
}

static void
test_remote_newline ()
{
	test_new_line (DEFAULT_REMOTE_URI, 0);
}

static void
test_remote ()
{
	test_saver (DEFAULT_REMOTE_URI,
	            "hello world",
	            GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	            0,
	            NULL,
	            saver_test_data_new (DEFAULT_REMOTE_URI, "hello world\n", NULL));

	test_saver (DEFAULT_REMOTE_URI,
	            "hello world\r\n",
	            GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	            0,
	            NULL,
	            saver_test_data_new (DEFAULT_REMOTE_URI, "hello world\n\n", NULL));

	test_saver (DEFAULT_REMOTE_URI,
	            "hello world\n",
	            GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	            0,
	            NULL,
	            saver_test_data_new (DEFAULT_REMOTE_URI, "hello world\n\n", NULL));
}

#ifndef G_OS_WIN32
static void
check_permissions (GFile *file,
                   guint  permissions)
{
	GError *error = NULL;
	GFileInfo *info;

	info = g_file_query_info (file,
	                          G_FILE_ATTRIBUTE_UNIX_MODE,
	                          G_FILE_QUERY_INFO_NONE,
	                          NULL,
	                          &error);

	g_assert_no_error (error);

	g_assert_cmpint (permissions,
	                 ==,
	                 g_file_info_get_attribute_uint32 (info, G_FILE_ATTRIBUTE_UNIX_MODE) & ACCESSPERMS);

	g_object_unref (info);
}

static void
check_permissions_saved (GeditDocument *document,
                         GError        *error,
                         SaverTestData *data)
{
	guint permissions = (guint)GPOINTER_TO_INT (data->data);
	GFile *file = gedit_document_get_location (document);

	check_permissions (file, permissions);

	g_object_unref (file);
}

static void
test_permissions (const gchar *uri,
                  guint        permissions)
{
	GError *error = NULL;
	GFile *file = g_file_new_for_commandline_arg (uri);
	GFileOutputStream *stream;
	GFileInfo *info;
	guint mode;

	g_file_delete (file, NULL, NULL);
	stream = g_file_create (file, 0, NULL, &error);

	g_assert_no_error (error);

	g_output_stream_close (G_OUTPUT_STREAM (stream), NULL, NULL);
	g_object_unref (stream);

	info = g_file_query_info (file,
	                          G_FILE_ATTRIBUTE_UNIX_MODE,
	                          G_FILE_QUERY_INFO_NONE,
	                          NULL,
	                          &error);

	g_assert_no_error (error);

	mode = g_file_info_get_attribute_uint32 (info, G_FILE_ATTRIBUTE_UNIX_MODE);
	g_object_unref (info);

	g_file_set_attribute_uint32 (file,
	                             G_FILE_ATTRIBUTE_UNIX_MODE,
	                             (mode & ~ACCESSPERMS) | permissions,
	                             G_FILE_QUERY_INFO_NONE,
	                             NULL,
	                             &error);
	g_assert_no_error (error);

	check_permissions (file, permissions);

	test_saver (uri,
	            DEFAULT_CONTENT,
	            GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	            0,
	            G_CALLBACK (check_permissions_saved),
	            saver_test_data_new (uri,
	                                 DEFAULT_CONTENT_RESULT,
	                                 GINT_TO_POINTER ((gint)permissions)));

	g_file_delete (file, NULL, NULL);
	g_object_unref (file);
}

static void
test_local_permissions ()
{
	test_permissions (DEFAULT_LOCAL_URI, 0600);
	test_permissions (DEFAULT_LOCAL_URI, 0660);
	test_permissions (DEFAULT_LOCAL_URI, 0666);
	test_permissions (DEFAULT_LOCAL_URI, 0760);
}
#endif

static void
test_local_unowned_directory ()
{
	test_saver (UNOWNED_LOCAL_URI,
	            DEFAULT_CONTENT,
	            GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	            0,
	            NULL,
	            saver_test_data_new (UNOWNED_LOCAL_URI,
	                                 DEFAULT_CONTENT_RESULT,
	                                 NULL));
}

static void
test_remote_unowned_directory ()
{
	test_saver (UNOWNED_REMOTE_URI,
	            DEFAULT_CONTENT,
	            GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	            0,
	            NULL,
	            saver_test_data_new (UNOWNED_REMOTE_URI,
	                                 DEFAULT_CONTENT_RESULT,
	                                 NULL));
}

#ifndef G_OS_WIN32
static void
test_remote_permissions ()
{
	test_permissions (DEFAULT_REMOTE_URI, 0600);
	test_permissions (DEFAULT_REMOTE_URI, 0660);
	test_permissions (DEFAULT_REMOTE_URI, 0666);
	test_permissions (DEFAULT_REMOTE_URI, 0760);
}

static void
test_unowned_group_permissions (GeditDocument *document,
                                GError        *error,
                                SaverTestData *data)
{
	GFile *file = g_file_new_for_commandline_arg (data->uri);
	GError *err = NULL;
	const gchar *group;
	guint32 mode;

	GFileInfo *info = g_file_query_info (file,
	                                     G_FILE_ATTRIBUTE_OWNER_GROUP ","
	                                     G_FILE_ATTRIBUTE_UNIX_MODE,
	                                     G_FILE_QUERY_INFO_NONE,
	                                     NULL,
	                                     &err);

	g_assert_no_error (err);

	group = g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_OWNER_GROUP);
	g_assert_cmpstr (group, ==, "root");

	mode = g_file_info_get_attribute_uint32 (info, G_FILE_ATTRIBUTE_UNIX_MODE);

	g_assert_cmpint (mode & ACCESSPERMS, ==, 0660);

	g_object_unref (file);
	g_object_unref (info);
}

static void
test_unowned_group (const gchar *uri)
{
	test_saver (uri,
	            DEFAULT_CONTENT,
	            GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	            0,
	            G_CALLBACK (test_unowned_group_permissions),
	            saver_test_data_new (uri,
	                                 DEFAULT_CONTENT_RESULT,
	                                 NULL));
}

static void
test_local_unowned_group ()
{
	test_unowned_group (UNOWNED_GROUP_LOCAL_URI);
}

static void
test_remote_unowned_group ()
{
	test_unowned_group (UNOWNED_GROUP_REMOTE_URI);
}

#endif

static gboolean
check_unowned_directory ()
{
	GFile *unowned = g_file_new_for_path (UNOWNED_LOCAL_DIRECTORY);
	GFile *unowned_file;
	GFileInfo *info;
	GError *error = NULL;

	g_printf ("*** Checking for unowned directory test... ");

	info = g_file_query_info (unowned,
	                          G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
	                          G_FILE_QUERY_INFO_NONE,
	                          NULL,
	                          &error);

	if (error)
	{
		g_object_unref (unowned);
		g_printf ("NO: directory does not exist\n");

		g_error_free (error);
		return FALSE;
	}

	if (g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
	{
		g_object_unref (unowned);

		g_printf ("NO: directory is writable\n");
		g_object_unref (info);
		return FALSE;
	}

	g_object_unref (info);
	g_object_unref (unowned);

	unowned_file = g_file_new_for_commandline_arg (UNOWNED_LOCAL_URI);

	info = g_file_query_info (unowned_file,
	                          G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
	                          G_FILE_QUERY_INFO_NONE,
	                          NULL,
	                          &error);

	if (error)
	{
		g_object_unref (unowned_file);
		g_error_free (error);

		g_printf ("NO: file does not exist\n");
		return FALSE;
	}

	if (!g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
	{
		g_object_unref (unowned_file);

		g_printf ("NO: file is not writable\n");
		g_object_unref (info);
		return FALSE;
	}

	g_object_unref (info);
	g_object_unref (unowned_file);

	g_printf ("YES\n");
	return TRUE;
}

static gboolean
check_unowned_group ()
{
	GFile *unowned = g_file_new_for_path (UNOWNED_GROUP_LOCAL_URI);
	GFileInfo *info;
	GError *error = NULL;

	g_printf ("*** Checking for unowned group test... ");

	info = g_file_query_info (unowned,
	                          G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE ","
	                          G_FILE_ATTRIBUTE_OWNER_GROUP ","
	                          G_FILE_ATTRIBUTE_UNIX_MODE,
	                          G_FILE_QUERY_INFO_NONE,
	                          NULL,
	                          &error);

	if (error)
	{
		g_object_unref (unowned);
		g_printf ("NO: file does not exist\n");

		g_error_free (error);
		return FALSE;
	}

	if (!g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
	{
		g_object_unref (unowned);

		g_printf ("NO: file is not writable\n");
		g_object_unref (info);
		return FALSE;
	}

	if (g_strcmp0 (g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_OWNER_GROUP),
	               "root") != 0)
	{
		g_object_unref (unowned);

		g_printf ("NO: group is not root (%s)\n", g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_OWNER_GROUP));
		g_object_unref (info);
		return FALSE;
	}

#ifndef G_OS_WIN32
	if ((g_file_info_get_attribute_uint32 (info, G_FILE_ATTRIBUTE_UNIX_MODE) & ACCESSPERMS) != 0660)
	{
		g_object_unref (unowned);

		g_printf ("NO: file has wrong permissions\n");
		g_object_unref (info);
		return FALSE;
	}
#endif

	g_object_unref (info);
	g_object_unref (unowned);

	g_printf ("YES\n");
	return TRUE;
}

int main (int   argc,
          char *argv[])
{
	gboolean have_unowned;
	gboolean have_unowned_group;

	g_type_init ();
	g_test_init (&argc, &argv, NULL);

	gedit_prefs_manager_app_init ();

	g_printf ("\n***\n");
	have_unowned = check_unowned_directory ();
	have_unowned_group = check_unowned_group ();
	g_printf ("***\n\n");

	g_test_add_func ("/document-saver/local", test_local);
	g_test_add_func ("/document-saver/local-new-line", test_local_newline);

	if (have_unowned)
	{
		g_test_add_func ("/document-saver/local-unowned-directory", test_local_unowned_directory);
	}

	g_test_add_func ("/document-saver/remote", test_remote);
	g_test_add_func ("/document-saver/remote-new-line", test_remote_newline);
	

	if (have_unowned)
	{
		g_test_add_func ("/document-saver/remote-unowned-directory", test_remote_unowned_directory);
	}

	if (have_unowned_group)
	{
		/* FIXME: there is a bug in gvfs sftp which doesn't pass this test */
		/* g_test_add_func ("/document-saver/remote-unowned-group", test_remote_unowned_group); */
	}

#ifndef G_OS_WIN32
	g_test_add_func ("/document-saver/local-permissions", test_local_permissions);

	if (have_unowned_group)
	{
		g_test_add_func ("/document-saver/local-unowned-group", test_local_unowned_group);
	}

	g_test_add_func ("/document-saver/remote-permissions", test_remote_permissions);
#endif

	return g_test_run ();
}
