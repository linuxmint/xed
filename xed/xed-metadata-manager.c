/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xed-metadata-manager.c
 * This file is part of xed
 *
 * Copyright (C) 2003-2007  Paolo Maggi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <libxml/globals.h>
#include <libxml/xmlreader.h>
#include "xed-metadata-manager.h"
#include "xed-debug.h"

/*
#define XED_METADATA_VERBOSE_DEBUG  1
*/

#define MAX_ITEMS   50

typedef struct _XedMetadataManager XedMetadataManager;

typedef struct _Item Item;

struct _Item
{
    gint64       atime; /* time of last access in seconds since January 1, 1970 UTC */

    GHashTable  *values;
};

struct _XedMetadataManager
{
    gboolean     values_loaded; /* It is true if the file
                       has been read */

    guint        timeout_id;

    GHashTable  *items;

    gchar *metadata_filename;
};

static gboolean xed_metadata_manager_save (gpointer data);


static XedMetadataManager *xed_metadata_manager = NULL;

static void
item_free (gpointer data)
{
    Item *item;

    g_return_if_fail (data != NULL);

#ifdef XED_METADATA_VERBOSE_DEBUG
    xed_debug (DEBUG_METADATA);
#endif

    item = (Item *)data;

    if (item->values != NULL)
        g_hash_table_destroy (item->values);

    g_free (item);
}

static void
xed_metadata_manager_arm_timeout (void)
{
    if (xed_metadata_manager->timeout_id == 0)
    {
        xed_metadata_manager->timeout_id =
            g_timeout_add_seconds_full (G_PRIORITY_DEFAULT_IDLE,
                            2,
                            (GSourceFunc)xed_metadata_manager_save,
                            NULL,
                            NULL);
    }
}

/**
 * xed_metadata_manager_init:
 * @metadata_filename: the filename where the metadata is.
 *
 * This function initializes the metadata manager.
 * See also xed_metadata_manager_shutdown().
 */
void
xed_metadata_manager_init (const gchar *metadata_filename)
{
    xed_debug (DEBUG_METADATA);

    if (xed_metadata_manager != NULL)
        return;

    xed_metadata_manager = g_new0 (XedMetadataManager, 1);

    xed_metadata_manager->values_loaded = FALSE;

    xed_metadata_manager->items =
        g_hash_table_new_full (g_str_hash,
                       g_str_equal,
                       g_free,
                       item_free);

    xed_metadata_manager->metadata_filename = g_strdup (metadata_filename);

    return;
}

/**
 * xed_metadata_manager_shutdown:
 *
 * This function frees the internal data of the metadata manager.
 * See also xed_metadata_manager_init().
 */
void
xed_metadata_manager_shutdown (void)
{
    xed_debug (DEBUG_METADATA);

    if (xed_metadata_manager == NULL)
        return;

    if (xed_metadata_manager->timeout_id)
    {
        g_source_remove (xed_metadata_manager->timeout_id);
        xed_metadata_manager->timeout_id = 0;
        xed_metadata_manager_save (NULL);
    }

    if (xed_metadata_manager->items != NULL)
        g_hash_table_destroy (xed_metadata_manager->items);

    g_free (xed_metadata_manager->metadata_filename);
    g_free (xed_metadata_manager);
    xed_metadata_manager = NULL;
}

static void
parseItem (xmlDocPtr doc, xmlNodePtr cur)
{
    Item *item;

    xmlChar *uri;
    xmlChar *atime;

#ifdef XED_METADATA_VERBOSE_DEBUG
    xed_debug (DEBUG_METADATA);
#endif

    if (xmlStrcmp (cur->name, (const xmlChar *)"document") != 0)
            return;

    uri = xmlGetProp (cur, (const xmlChar *)"uri");
    if (uri == NULL)
        return;

    atime = xmlGetProp (cur, (const xmlChar *)"atime");
    if (atime == NULL)
    {
        xmlFree (uri);
        return;
    }

    item = g_new0 (Item, 1);

    item->atime = g_ascii_strtoll ((char *)atime, NULL, 0);

    item->values = g_hash_table_new_full (g_str_hash,
                          g_str_equal,
                          g_free,
                          g_free);

    cur = cur->xmlChildrenNode;

    while (cur != NULL)
    {
        if (xmlStrcmp (cur->name, (const xmlChar *)"entry") == 0)
        {
            xmlChar *key;
            xmlChar *value;

            key = xmlGetProp (cur, (const xmlChar *)"key");
            value = xmlGetProp (cur, (const xmlChar *)"value");

            if ((key != NULL) && (value != NULL))
                g_hash_table_insert (item->values,
                             g_strdup ((gchar *)key),
                             g_strdup ((gchar *)value));

            if (key != NULL)
                xmlFree (key);
            if (value != NULL)
                xmlFree (value);
        }

        cur = cur->next;
    }

    g_hash_table_insert (xed_metadata_manager->items,
                 g_strdup ((gchar *)uri),
                 item);

    xmlFree (uri);
    xmlFree (atime);
}

static gboolean
load_values (void)
{
    xmlDocPtr doc;
    xmlNodePtr cur;

    xed_debug (DEBUG_METADATA);

    g_return_val_if_fail (xed_metadata_manager != NULL, FALSE);
    g_return_val_if_fail (xed_metadata_manager->values_loaded == FALSE, FALSE);

    xed_metadata_manager->values_loaded = TRUE;

    xmlKeepBlanksDefault (0);

    /* FIXME: file locking - Paolo */
    if (xed_metadata_manager->metadata_filename == NULL)
    {
        return FALSE;
    }

    /* TODO: avoid races */
    if (!g_file_test (xed_metadata_manager->metadata_filename, G_FILE_TEST_EXISTS))
    {
        return TRUE;
    }

    doc = xmlParseFile (xed_metadata_manager->metadata_filename);

    if (doc == NULL)
    {
        return FALSE;
    }

    cur = xmlDocGetRootElement (doc);
    if (cur == NULL)
    {
        g_message ("The metadata file '%s' is empty", g_path_get_basename (xed_metadata_manager->metadata_filename));
        xmlFreeDoc (doc);

        return TRUE;
    }

    if (xmlStrcmp (cur->name, (const xmlChar *) "metadata"))
    {
        g_message ("File '%s' is of the wrong type", g_path_get_basename (xed_metadata_manager->metadata_filename));
        xmlFreeDoc (doc);

        return FALSE;
    }

    cur = xmlDocGetRootElement (doc);
    cur = cur->xmlChildrenNode;

    while (cur != NULL)
    {
        parseItem (doc, cur);

        cur = cur->next;
    }

    xmlFreeDoc (doc);

    return TRUE;
}

/**
 * xed_metadata_manager_get:
 * @location: a #GFile.
 * @key: a key.
 *
 * Gets the value associated with the specified @key for the file @location.
 */
gchar *
xed_metadata_manager_get (GFile *location,
                const gchar *key)
{
    Item *item;
    gchar *value;
    gchar *uri;

    g_return_val_if_fail (G_IS_FILE (location), NULL);
    g_return_val_if_fail (key != NULL, NULL);

    uri = g_file_get_uri (location);

    xed_debug_message (DEBUG_METADATA, "URI: %s --- key: %s", uri, key );

    if (!xed_metadata_manager->values_loaded)
    {
        gboolean res;

        res = load_values ();

        if (!res)
        {
            g_free (uri);
            return NULL;
        }
    }

    item = (Item *)g_hash_table_lookup (xed_metadata_manager->items,
                        uri);

    g_free (uri);

    if (item == NULL)
        return NULL;

    item->atime = g_get_real_time () / 1000;

    if (item->values == NULL)
        return NULL;

    value = g_hash_table_lookup (item->values, key);

    if (value == NULL)
        return NULL;
    else
        return g_strdup (value);
}

/**
 * xed_metadata_manager_set:
 * @location: a #GFile.
 * @key: a key.
 * @value: the value associated with the @key.
 *
 * Sets the @key to contain the given @value for the file @location.
 */
void
xed_metadata_manager_set (GFile *location,
                const gchar *key,
                const gchar *value)
{
    Item *item;
    gchar *uri;

    g_return_if_fail (G_IS_FILE (location));
    g_return_if_fail (key != NULL);

    uri = g_file_get_uri (location);

    xed_debug_message (DEBUG_METADATA, "URI: %s --- key: %s --- value: %s", uri, key, value);

    if (!xed_metadata_manager->values_loaded)
    {
        gboolean ok;

        ok = load_values ();

        if (!ok)
        {
            g_free (uri);
            return;
        }
    }

    item = (Item *)g_hash_table_lookup (xed_metadata_manager->items,
                        uri);

    if (item == NULL)
    {
        item = g_new0 (Item, 1);

        g_hash_table_insert (xed_metadata_manager->items,
                     g_strdup (uri),
                     item);
    }

    if (item->values == NULL)
         item->values = g_hash_table_new_full (g_str_hash,
                               g_str_equal,
                               g_free,
                               g_free);
    if (value != NULL)
        g_hash_table_insert (item->values,
                     g_strdup (key),
                     g_strdup (value));
    else
        g_hash_table_remove (item->values,
                     key);

    item->atime = g_get_real_time () / 1000;

    g_free (uri);

    xed_metadata_manager_arm_timeout ();
}

static void
save_values (const gchar *key, const gchar *value, xmlNodePtr parent)
{
    xmlNodePtr xml_node;

#ifdef XED_METADATA_VERBOSE_DEBUG
    xed_debug (DEBUG_METADATA);
#endif

    g_return_if_fail (key != NULL);

    if (value == NULL)
        return;

    xml_node = xmlNewChild (parent,
                NULL,
                (const xmlChar *)"entry",
                NULL);

    xmlSetProp (xml_node,
            (const xmlChar *)"key",
            (const xmlChar *)key);
    xmlSetProp (xml_node,
            (const xmlChar *)"value",
            (const xmlChar *)value);

#ifdef XED_METADATA_VERBOSE_DEBUG
    xed_debug_message (DEBUG_METADATA, "entry: %s = %s", key, value);
#endif
}

static void
save_item (const gchar *key, const gpointer *data, xmlNodePtr parent)
{
    xmlNodePtr xml_node;
    const Item *item = (const Item *)data;
    gchar *atime;

#ifdef XED_METADATA_VERBOSE_DEBUG
    xed_debug (DEBUG_METADATA);
#endif

    g_return_if_fail (key != NULL);

    if (item == NULL)
        return;

    xml_node = xmlNewChild (parent, NULL, (const xmlChar *)"document", NULL);

    xmlSetProp (xml_node, (const xmlChar *)"uri", (const xmlChar *)key);

#ifdef XED_METADATA_VERBOSE_DEBUG
    xed_debug_message (DEBUG_METADATA, "uri: %s", key);
#endif

    atime = g_strdup_printf ("%" G_GINT64_FORMAT, item->atime);
    xmlSetProp (xml_node, (const xmlChar *)"atime", (const xmlChar *)atime);

#ifdef XED_METADATA_VERBOSE_DEBUG
    xed_debug_message (DEBUG_METADATA, "atime: %s", atime);
#endif

    g_free (atime);

        g_hash_table_foreach (item->values,
                  (GHFunc)save_values,
                  xml_node);
}

static void
get_oldest (const gchar *key, const gpointer value, const gchar ** key_to_remove)
{
    const Item *item = (const Item *)value;

    if (*key_to_remove == NULL)
    {
        *key_to_remove = key;
    }
    else
    {
        const Item *item_to_remove =
            g_hash_table_lookup (xed_metadata_manager->items,
                         *key_to_remove);

        g_return_if_fail (item_to_remove != NULL);

        if (item->atime < item_to_remove->atime)
        {
            *key_to_remove = key;
        }
    }
}

static void
resize_items (void)
{
    while (g_hash_table_size (xed_metadata_manager->items) > MAX_ITEMS)
    {
        gpointer key_to_remove = NULL;

        g_hash_table_foreach (xed_metadata_manager->items,
                      (GHFunc)get_oldest,
                      &key_to_remove);

        g_return_if_fail (key_to_remove != NULL);

        g_hash_table_remove (xed_metadata_manager->items,
                     key_to_remove);
    }
}

static gboolean
xed_metadata_manager_save (gpointer data)
{
    xmlDocPtr  doc;
    xmlNodePtr root;

    xed_debug (DEBUG_METADATA);

    xed_metadata_manager->timeout_id = 0;

    resize_items ();

    xmlIndentTreeOutput = TRUE;

    doc = xmlNewDoc ((const xmlChar *)"1.0");
    if (doc == NULL)
        return TRUE;

    /* Create metadata root */
    root = xmlNewDocNode (doc, NULL, (const xmlChar *)"metadata", NULL);
    xmlDocSetRootElement (doc, root);

        g_hash_table_foreach (xed_metadata_manager->items,
                  (GHFunc)save_item,
                  root);

    /* FIXME: lock file - Paolo */
    if (xed_metadata_manager->metadata_filename != NULL)
    {
        gchar *cache_dir;
        int res;

        /* make sure the cache dir exists */
        cache_dir = g_path_get_dirname (xed_metadata_manager->metadata_filename);
        res = g_mkdir_with_parents (cache_dir, 0755);
        if (res != -1)
        {
            xmlSaveFormatFile (xed_metadata_manager->metadata_filename, doc, 1);
        }

        g_free (cache_dir);
    }

    xmlFreeDoc (doc);

    xed_debug_message (DEBUG_METADATA, "DONE");

    return FALSE;
}
