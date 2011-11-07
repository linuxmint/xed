/*
 * gedit-history-entry.h
 * This file is part of gedit
 *
 * Copyright (C) 2006 - Paolo Borelli
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
 * Modified by the gedit Team, 2006. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_HISTORY_ENTRY_H__
#define __GEDIT_HISTORY_ENTRY_H__


G_BEGIN_DECLS

#define GEDIT_TYPE_HISTORY_ENTRY             (gedit_history_entry_get_type ())
#define GEDIT_HISTORY_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_HISTORY_ENTRY, GeditHistoryEntry))
#define GEDIT_HISTORY_ENTRY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_HISTORY_ENTRY, GeditHistoryEntryClass))
#define GEDIT_IS_HISTORY_ENTRY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_HISTORY_ENTRY))
#define GEDIT_IS_HISTORY_ENTRY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_HISTORY_ENTRY))
#define GEDIT_HISTORY_ENTRY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_HISTORY_ENTRY, GeditHistoryEntryClass))


typedef struct _GeditHistoryEntry        GeditHistoryEntry;
typedef struct _GeditHistoryEntryClass   GeditHistoryEntryClass;
typedef struct _GeditHistoryEntryPrivate GeditHistoryEntryPrivate;

struct _GeditHistoryEntryClass
{
	GtkComboBoxEntryClass parent_class;
};

struct _GeditHistoryEntry
{
	GtkComboBoxEntry parent_instance;

	GeditHistoryEntryPrivate *priv;
};

GType		 gedit_history_entry_get_type	(void) G_GNUC_CONST;

GtkWidget	*gedit_history_entry_new		(const gchar       *history_id,
							 gboolean           enable_completion);

void		 gedit_history_entry_prepend_text	(GeditHistoryEntry *entry,
							 const gchar       *text);

void		 gedit_history_entry_append_text	(GeditHistoryEntry *entry,
							 const gchar       *text);

void		 gedit_history_entry_clear		(GeditHistoryEntry *entry);

void		 gedit_history_entry_set_history_length	(GeditHistoryEntry *entry,
							 guint              max_saved);

guint		 gedit_history_entry_get_history_length	(GeditHistoryEntry *gentry);

gchar		*gedit_history_entry_get_history_id	(GeditHistoryEntry *entry);

void             gedit_history_entry_set_enable_completion 
							(GeditHistoryEntry *entry,
							 gboolean           enable);
							 
gboolean         gedit_history_entry_get_enable_completion 
							(GeditHistoryEntry *entry);

GtkWidget	*gedit_history_entry_get_entry		(GeditHistoryEntry *entry);

typedef gchar * (* GeditHistoryEntryEscapeFunc) (const gchar *str);
void		gedit_history_entry_set_escape_func	(GeditHistoryEntry *entry,
							 GeditHistoryEntryEscapeFunc escape_func);

G_END_DECLS

#endif /* __GEDIT_HISTORY_ENTRY_H__ */
