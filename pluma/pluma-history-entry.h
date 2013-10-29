/*
 * pluma-history-entry.h
 * This file is part of pluma
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */
 
/*
 * Modified by the pluma Team, 2006. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_HISTORY_ENTRY_H__
#define __PLUMA_HISTORY_ENTRY_H__


G_BEGIN_DECLS

#define PLUMA_TYPE_HISTORY_ENTRY             (pluma_history_entry_get_type ())
#define PLUMA_HISTORY_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_HISTORY_ENTRY, PlumaHistoryEntry))
#define PLUMA_HISTORY_ENTRY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_HISTORY_ENTRY, PlumaHistoryEntryClass))
#define PLUMA_IS_HISTORY_ENTRY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_HISTORY_ENTRY))
#define PLUMA_IS_HISTORY_ENTRY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_HISTORY_ENTRY))
#define PLUMA_HISTORY_ENTRY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_HISTORY_ENTRY, PlumaHistoryEntryClass))


typedef struct _PlumaHistoryEntry        PlumaHistoryEntry;
typedef struct _PlumaHistoryEntryClass   PlumaHistoryEntryClass;
typedef struct _PlumaHistoryEntryPrivate PlumaHistoryEntryPrivate;

struct _PlumaHistoryEntryClass
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkComboBoxTextClass parent_class;
#else
	GtkComboBoxEntryClass parent_class;
#endif
};

struct _PlumaHistoryEntry
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkComboBoxText parent_instance;
#else
	GtkComboBoxEntry parent_instance;
#endif

	PlumaHistoryEntryPrivate *priv;
};

GType		 pluma_history_entry_get_type	(void) G_GNUC_CONST;

GtkWidget	*pluma_history_entry_new		(const gchar       *history_id,
							 gboolean           enable_completion);

void		 pluma_history_entry_prepend_text	(PlumaHistoryEntry *entry,
							 const gchar       *text);

void		 pluma_history_entry_append_text	(PlumaHistoryEntry *entry,
							 const gchar       *text);

void		 pluma_history_entry_clear		(PlumaHistoryEntry *entry);

void		 pluma_history_entry_set_history_length	(PlumaHistoryEntry *entry,
							 guint              max_saved);

guint		 pluma_history_entry_get_history_length	(PlumaHistoryEntry *gentry);

gchar		*pluma_history_entry_get_history_id	(PlumaHistoryEntry *entry);

void             pluma_history_entry_set_enable_completion 
							(PlumaHistoryEntry *entry,
							 gboolean           enable);
							 
gboolean         pluma_history_entry_get_enable_completion 
							(PlumaHistoryEntry *entry);

GtkWidget	*pluma_history_entry_get_entry		(PlumaHistoryEntry *entry);

typedef gchar * (* PlumaHistoryEntryEscapeFunc) (const gchar *str);
void		pluma_history_entry_set_escape_func	(PlumaHistoryEntry *entry,
							 PlumaHistoryEntryEscapeFunc escape_func);

G_END_DECLS

#endif /* __PLUMA_HISTORY_ENTRY_H__ */
