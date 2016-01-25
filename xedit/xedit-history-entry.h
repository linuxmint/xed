/*
 * xedit-history-entry.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 2006. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XEDIT_HISTORY_ENTRY_H__
#define __XEDIT_HISTORY_ENTRY_H__


G_BEGIN_DECLS

#define XEDIT_TYPE_HISTORY_ENTRY             (xedit_history_entry_get_type ())
#define XEDIT_HISTORY_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_HISTORY_ENTRY, XeditHistoryEntry))
#define XEDIT_HISTORY_ENTRY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_HISTORY_ENTRY, XeditHistoryEntryClass))
#define XEDIT_IS_HISTORY_ENTRY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_HISTORY_ENTRY))
#define XEDIT_IS_HISTORY_ENTRY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_HISTORY_ENTRY))
#define XEDIT_HISTORY_ENTRY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_HISTORY_ENTRY, XeditHistoryEntryClass))


typedef struct _XeditHistoryEntry        XeditHistoryEntry;
typedef struct _XeditHistoryEntryClass   XeditHistoryEntryClass;
typedef struct _XeditHistoryEntryPrivate XeditHistoryEntryPrivate;

struct _XeditHistoryEntryClass
{
	GtkComboBoxTextClass parent_class;
};

struct _XeditHistoryEntry
{
	GtkComboBoxText parent_instance;

	XeditHistoryEntryPrivate *priv;
};

GType		 xedit_history_entry_get_type	(void) G_GNUC_CONST;

GtkWidget	*xedit_history_entry_new		(const gchar       *history_id,
							 gboolean           enable_completion);

void		 xedit_history_entry_prepend_text	(XeditHistoryEntry *entry,
							 const gchar       *text);

void		 xedit_history_entry_append_text	(XeditHistoryEntry *entry,
							 const gchar       *text);

void		 xedit_history_entry_clear		(XeditHistoryEntry *entry);

void		 xedit_history_entry_set_history_length	(XeditHistoryEntry *entry,
							 guint              max_saved);

guint		 xedit_history_entry_get_history_length	(XeditHistoryEntry *gentry);

gchar		*xedit_history_entry_get_history_id	(XeditHistoryEntry *entry);

void             xedit_history_entry_set_enable_completion 
							(XeditHistoryEntry *entry,
							 gboolean           enable);
							 
gboolean         xedit_history_entry_get_enable_completion 
							(XeditHistoryEntry *entry);

GtkWidget	*xedit_history_entry_get_entry		(XeditHistoryEntry *entry);

typedef gchar * (* XeditHistoryEntryEscapeFunc) (const gchar *str);
void		xedit_history_entry_set_escape_func	(XeditHistoryEntry *entry,
							 XeditHistoryEntryEscapeFunc escape_func);

G_END_DECLS

#endif /* __XEDIT_HISTORY_ENTRY_H__ */
