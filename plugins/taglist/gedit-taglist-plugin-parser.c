/*
 * gedit-taglist-plugin-parser.c
 * This file is part of gedit
 *
 * Copyright (C) 2002-2005 - Paolo Maggi 
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */
 
/*
 * Modified by the gedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

/* FIXME: we should rewrite the parser to avoid using DOM */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <libxml/parser.h>
#include <glib.h>
#include <glib/gi18n.h>

#include <gedit/gedit-debug.h>

#include "gedit-taglist-plugin-parser.h"

/* we screwed up so we still look here for compatibility */
#define USER_GEDIT_TAGLIST_PLUGIN_LOCATION_LEGACY ".gedit-2/plugins/taglist/"
#define USER_GEDIT_TAGLIST_PLUGIN_LOCATION "gedit/taglist/"

TagList *taglist = NULL;
static gint taglist_ref_count = 0;

static gboolean	 parse_tag (Tag *tag, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static gboolean	 parse_tag_group (TagGroup *tg, const gchar *fn, 
				  xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur,
				  gboolean sort);
static TagGroup* get_tag_group (const gchar* filename, xmlDocPtr doc, 
				xmlNsPtr ns, xmlNodePtr cur);
static TagList* lookup_best_lang (TagList *taglist, const gchar *filename, 
				xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static TagList 	*parse_taglist_file (const gchar* filename);
static TagList  *parse_taglist_dir (const gchar *dir);

static void	 free_tag (Tag *tag);
static void	 free_tag_group (TagGroup *tag_group);

static gboolean
parse_tag (Tag *tag, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
	/*
	gedit_debug_message (DEBUG_PLUGINS, "  Tag name: %s", tag->name);
	*/
	/* We don't care what the top level element name is */
	cur = cur->xmlChildrenNode;
    
	while (cur != NULL) 
	{
		if ((!xmlStrcmp (cur->name, (const xmlChar *)"Begin")) &&
		    (cur->ns == ns))
		{			
			tag->begin = xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			/*
			gedit_debug_message (DEBUG_PLUGINS, "    - Begin: %s", tag->begin);
			*/
		}

		if ((!xmlStrcmp (cur->name, (const xmlChar *)"End")) &&
		    (cur->ns == ns))			
		{
			tag->end = xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			/*
			gedit_debug_message (DEBUG_PLUGINS, "    - End: %s", tag->end);
			*/
		}

		cur = cur->next;
	}

	if ((tag->begin == NULL) && (tag->end == NULL))
		return FALSE;

	return TRUE;
}

static gint
tags_cmp (gconstpointer a, gconstpointer b)
{
	gchar *tag_a = (gchar*)((Tag *)a)->name;
	gchar *tag_b = (gchar*)((Tag *)b)->name;
	
	return g_utf8_collate (tag_a, tag_b);
}

static gboolean
parse_tag_group (TagGroup *tg, const gchar* fn, xmlDocPtr doc, 
		 xmlNsPtr ns, xmlNodePtr cur, gboolean sort) 
{
	gedit_debug_message (DEBUG_PLUGINS, "Parse TagGroup: %s", tg->name);

	/* We don't care what the top level element name is */
    	cur = cur->xmlChildrenNode;
    
	while (cur != NULL) 
	{
		if ((xmlStrcmp (cur->name, (const xmlChar *) "Tag")) || (cur->ns != ns)) 
		{
			g_warning ("The tag list file '%s' is of the wrong type, "
				   "was '%s', 'Tag' expected.", fn, cur->name);
				
			return FALSE;
		}
		else
		{
			Tag *tag;

			tag = g_new0 (Tag, 1);

			/* Get Tag name */
			tag->name = xmlGetProp (cur, (const xmlChar *) "name");

			if (tag->name == NULL)
			{
				/* Error: No name */
				g_warning ("The tag list file '%s' is of the wrong type, "
				   "Tag without name.", fn);

				g_free (tag);

				return FALSE;
			}
			else
			{
				/* Parse Tag */
				if (parse_tag (tag, doc, ns, cur))
				{
					/* Prepend Tag to TagGroup */
					tg->tags = g_list_prepend (tg->tags, tag);
				}
				else
				{
					/* Error parsing Tag */
					g_warning ("The tag list file '%s' is of the wrong type, "
			   		   	   "error parsing Tag '%s' in TagGroup '%s'.", 
					   	   fn, tag->name, tg->name);
	
					free_tag (tag);
					
					return FALSE;
				}
			}
		}

		cur = cur->next;		
	}

	if (sort)
		tg->tags = g_list_sort (tg->tags, tags_cmp);
	else
		tg->tags = g_list_reverse (tg->tags);	
	
	return TRUE;
}

static TagGroup*
get_tag_group (const gchar* filename, xmlDocPtr doc, 
		 xmlNsPtr ns, xmlNodePtr cur) 
{
	TagGroup *tag_group;
	xmlChar *sort_str;
	gboolean sort = FALSE;

	tag_group = g_new0 (TagGroup, 1);

	/* Get TagGroup name */
	tag_group->name = xmlGetProp (cur, (const xmlChar *) "name");

	sort_str = xmlGetProp (cur, (const xmlChar *) "sort");

	if ((sort_str != NULL) &&
	    ((xmlStrcasecmp (sort_str, (const xmlChar *) "yes") == 0) ||
	     (xmlStrcasecmp (sort_str, (const xmlChar *) "true") == 0) ||
	     (xmlStrcasecmp (sort_str, (const xmlChar *) "1") == 0)))
	{
		sort = TRUE;
	}

	xmlFree(sort_str);

	if (tag_group->name == NULL)
	{
		/* Error: No name */
		g_warning ("The tag list file '%s' is of the wrong type, "
		   "TagGroup without name.", filename);

		g_free (tag_group);
	}
	else
	{
		/* Name found */
		gboolean exists = FALSE;
		GList *t = taglist->tag_groups;
		
		/* Check if the tag group already exists */
		while (t && !exists)
		{
			gchar *tgn = (gchar*)((TagGroup*)(t->data))->name;
			
			if (strcmp (tgn, (gchar*)tag_group->name) == 0)
			{
				gedit_debug_message (DEBUG_PLUGINS, 
					     "Tag group '%s' already exists.", tgn);
				
				exists = TRUE;

				free_tag_group (tag_group);
			}
			
			t = g_list_next (t);		
		}

		if (!exists)
		{				
			/* Parse tag group */
			if (parse_tag_group (tag_group, filename, doc, ns, cur, sort))
			{
				return tag_group;
			}
			else
			{
				/* Error parsing TagGroup */
				g_warning ("The tag list file '%s' is of the wrong type, "
		   			   "error parsing TagGroup '%s'.", 
					   filename, tag_group->name);

				free_tag_group (tag_group);
			}
		}
	}
	return NULL;
}

static gint
groups_cmp (gconstpointer a, gconstpointer b)
{
	gchar *g_a = (gchar *)((TagGroup *)a)->name;
	gchar *g_b = (gchar *)((TagGroup *)b)->name;
	
	return g_utf8_collate (g_a, g_b);
}

/* 
 *  tags file is localized by intltool-merge below.
 *
 *      <gedit:TagGroup name="XSLT - Elements">
 *      </gedit:TagGroup>
 *      <gedit:TagGroup xml:lang="am" name="LOCALIZED TEXT">
 *      </gedit:TagGroup>
 *      <gedit:TagGroup xml:lang="ar" name="LOCALIZED TEXT">
 *      </gedit:TagGroup>
 *      .....
 *      <gedit:TagGroup name="XSLT - Functions">
 *      </gedit:TagGroup>
 *      .....
 *  Therefore need to pick up the best lang on the current locale.
 */
static TagList*
lookup_best_lang (TagList *taglist, const gchar *filename, 
		xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{

	TagGroup *best_tag_group = NULL;
	TagGroup *tag_group;
	gint best_lanking = -1;

	/*
	 * Walk the tree.
	 *
	 * First level we expect a list TagGroup 
	 */
	cur = cur->xmlChildrenNode;
	
	while (cur != NULL)
     	{ 	
		if ((xmlStrcmp (cur->name, (const xmlChar *) "TagGroup")) || (cur->ns != ns)) 
		{
			g_warning ("The tag list file '%s' is of the wrong type, "
				   "was '%s', 'TagGroup' expected.", filename, cur->name);
			xmlFreeDoc (doc);

			return taglist;
		}
		else
		{
			const char * const *langs_pointer;
			gchar *lang;
			gint cur_lanking;
			gint i;

			langs_pointer = g_get_language_names ();

			lang = (gchar*) xmlGetProp (cur, (const xmlChar*) "lang");
			cur_lanking = 1;

			/* 
			 * When found a new TagGroup, prepend the best 
			 * tag_group to taglist. In the current intltool-merge, 
			 * the first section is the default lang NULL.
			 */
			if (lang == NULL) {
				if (best_tag_group != NULL) {
					taglist->tag_groups = 
					g_list_prepend (taglist->tag_groups, best_tag_group);
				}

				best_tag_group = NULL;
				best_lanking = -1;
			}

			/* 
			 * If already find the best TagGroup on the current 
			 * locale, ignore the logic.
			 */
			if (best_lanking != -1 && best_lanking <= cur_lanking) {
				cur = cur->next;
				continue;
			}

			/* try to find the best lang */
			for (i = 0; langs_pointer[i] != NULL; i++)
			{
				const gchar *best_lang = langs_pointer[i];

				/*
				 * if launch on C, POSIX locale or does 
				 * not find the best lang on the current locale,
				 * this is called.
				 * g_get_language_names returns lang 
				 * lists with C locale.
				 */
				if (lang == NULL &&
				    (!g_ascii_strcasecmp (best_lang, "C") || 
				     !g_ascii_strcasecmp (best_lang, "POSIX")))
				{
					tag_group = get_tag_group (filename, doc, ns, cur);
					if (tag_group != NULL)
					{
						if (best_tag_group !=NULL) 
							free_tag_group (best_tag_group);
						best_lanking = cur_lanking;
						best_tag_group = tag_group;
					}
				}

				/* if it is possible the best lang is not C */
				else if (lang == NULL)
				{
					cur_lanking++;
					continue;
				}

				/* if the best lang is found */
				else if (!g_ascii_strcasecmp (best_lang, lang))
				{
					tag_group = get_tag_group (filename, doc, ns, cur);
					if (tag_group != NULL)
					{
						if (best_tag_group !=NULL) 
							free_tag_group (best_tag_group);
						best_lanking = cur_lanking;
						best_tag_group = tag_group;
					}
				}

				cur_lanking++;
			}

			if (lang) g_free (lang);
		} /* End of else */
	
		cur = cur->next;
    	} /* End of while (cur != NULL) */

	/* Prepend TagGroup to TagList */
	if (best_tag_group != NULL) {
		taglist->tag_groups = 
			g_list_prepend (taglist->tag_groups, best_tag_group);
	}

	taglist->tag_groups = g_list_sort (taglist->tag_groups, groups_cmp);
	
	return taglist;
}

static TagList *
parse_taglist_file (const gchar* filename)
{
	xmlDocPtr doc;
	
	xmlNsPtr ns;
	xmlNodePtr cur;

	gedit_debug_message (DEBUG_PLUGINS, "Parse file: %s", filename);

	xmlKeepBlanksDefault (0);

	/*
	* build an XML tree from a the file;
	*/
	doc = xmlParseFile (filename);
	if (doc == NULL) 
	{	
		g_warning ("The tag list file '%s' is empty.", filename);	
	
		return taglist;
	}

	/*
	* Check the document is of the right kind
	*/
    
	cur = xmlDocGetRootElement (doc);

	if (cur == NULL) 
	{
		g_warning ("The tag list file '%s' is empty.", filename);		
		xmlFreeDoc(doc);		
		return taglist;
	}

	ns = xmlSearchNsByHref (doc, cur,
			(const xmlChar *) "http://gedit.sourceforge.net/some-location");

	if (ns == NULL) 
	{
		g_warning ("The tag list file '%s' is of the wrong type, "
			   "gedit namespace not found.", filename);
		xmlFreeDoc (doc);
		
		return taglist;
	}

    	if (xmlStrcmp(cur->name, (const xmlChar *) "TagList")) 
	{
		g_warning ("The tag list file '%s' is of the wrong type, "
			   "root node != TagList.", filename);
		xmlFreeDoc (doc);
		
		return taglist;
	}

	/* 
	 * If needed, allocate taglist
	 */

	if (taglist == NULL)
		taglist = g_new0 (TagList, 1);
	
	taglist = lookup_best_lang (taglist, filename, doc, ns, cur);

	xmlFreeDoc (doc);

	gedit_debug_message (DEBUG_PLUGINS, "END");

	return taglist;
}

static void
free_tag (Tag *tag)
{
	/*
	gedit_debug_message (DEBUG_PLUGINS, "Tag: %s", tag->name);
	*/
	g_return_if_fail (tag != NULL);

	free (tag->name);

	if (tag->begin != NULL)
		free (tag->begin);

	if (tag->end != NULL)
		free (tag->end);

	g_free (tag);
}

static void
free_tag_group (TagGroup *tag_group)
{
	GList *l;

	gedit_debug_message (DEBUG_PLUGINS, "Tag group: %s", tag_group->name);

	g_return_if_fail (tag_group != NULL);

	free (tag_group->name);

	for (l = tag_group->tags; l != NULL; l = g_list_next (l))
	{
		free_tag ((Tag *) l->data);
	}

	g_list_free (tag_group->tags);
	g_free (tag_group);

	gedit_debug_message (DEBUG_PLUGINS, "END");
}

void
free_taglist (void)
{
	GList *l;

	gedit_debug_message (DEBUG_PLUGINS, "ref_count: %d", taglist_ref_count);

	if (taglist == NULL)
		return;

	g_return_if_fail (taglist_ref_count > 0);

	--taglist_ref_count;
	if (taglist_ref_count > 0)
		return;

	for (l = taglist->tag_groups; l != NULL; l = g_list_next (l))
	{
		free_tag_group ((TagGroup *) l->data);
	}

	g_list_free (taglist->tag_groups);
	g_free (taglist);
	taglist = NULL;

	gedit_debug_message (DEBUG_PLUGINS, "Really freed");
}

static TagList * 
parse_taglist_dir (const gchar *dir)
{
	GError *error = NULL;
	GDir *d;
	const gchar *dirent;

	gedit_debug_message (DEBUG_PLUGINS, "DIR: %s", dir);

	d = g_dir_open (dir, 0, &error);
	if (!d)
	{
		gedit_debug_message (DEBUG_PLUGINS, "%s", error->message);
		g_error_free (error);
		return taglist;
	}

	while ((dirent = g_dir_read_name (d)))
	{
		if (g_str_has_suffix (dirent, ".tags") ||
		    g_str_has_suffix (dirent, ".tags.gz"))
		{
			gchar *tags_file = g_build_filename (dir, dirent, NULL);
			parse_taglist_file (tags_file);
			g_free (tags_file);
		}
	}

	g_dir_close (d);

	return taglist;
}

TagList* create_taglist (const gchar *data_dir)
{
	gchar *pdir;

	gedit_debug_message (DEBUG_PLUGINS, "ref_count: %d", taglist_ref_count);

	if (taglist_ref_count > 0)
	{		
		++taglist_ref_count;
		
		return taglist;
	}

#ifndef G_OS_WIN32
	const gchar *home;
	const gchar *envvar;

	/* load user's taglists */

	/* legacy dir */
	home = g_get_home_dir ();
	if (home != NULL)
	{
		pdir = g_build_filename (home,
					 USER_GEDIT_TAGLIST_PLUGIN_LOCATION_LEGACY,
					 NULL);
		parse_taglist_dir (pdir);
		g_free (pdir);
	}

	/* Support old libmate env var */
	envvar = g_getenv ("MATE22_USER_DIR");
	if (envvar != NULL)
	{
		pdir = g_build_filename (envvar,
					 USER_GEDIT_TAGLIST_PLUGIN_LOCATION,
					 NULL);
		parse_taglist_dir (pdir);
		g_free (pdir);
	}
	else if (home != NULL)
	{
		pdir = g_build_filename (home,
					 ".mate2",
					 USER_GEDIT_TAGLIST_PLUGIN_LOCATION,
					 NULL);
		parse_taglist_dir (pdir);
		g_free (pdir);
	}

#else	
	pdir = g_build_filename (g_get_user_config_dir (),
				 "gedit",
				 "taglist",
				 NULL);
	parse_taglist_dir (pdir);
	g_free (pdir);
#endif
	
	/* load system's taglists */
	parse_taglist_dir (data_dir);

	++taglist_ref_count;
	g_return_val_if_fail (taglist_ref_count == 1, taglist);
	
	return taglist;
}
