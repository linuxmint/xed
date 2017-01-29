/*
 * xed-history-entry.h
 * This file is part of xed
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
 * Modified by the xed Team, 2006. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_HISTORY_ENTRY_H__
#define __XED_HISTORY_ENTRY_H__


G_BEGIN_DECLS

#define XED_TYPE_HISTORY_ENTRY             (xed_history_entry_get_type ())
#define XED_HISTORY_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_HISTORY_ENTRY, XedHistoryEntry))
#define XED_HISTORY_ENTRY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_HISTORY_ENTRY, XedHistoryEntryClass))
#define XED_IS_HISTORY_ENTRY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_HISTORY_ENTRY))
#define XED_IS_HISTORY_ENTRY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_HISTORY_ENTRY))
#define XED_HISTORY_ENTRY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_HISTORY_ENTRY, XedHistoryEntryClass))


typedef struct _XedHistoryEntry        XedHistoryEntry;
typedef struct _XedHistoryEntryClass   XedHistoryEntryClass;
typedef struct _XedHistoryEntryPrivate XedHistoryEntryPrivate;

struct _XedHistoryEntryClass
{
    GtkComboBoxTextClass parent_class;
};

struct _XedHistoryEntry
{
    GtkComboBoxText parent_instance;

    XedHistoryEntryPrivate *priv;
};

GType xed_history_entry_get_type (void) G_GNUC_CONST;

GtkWidget *xed_history_entry_new (const gchar *history_id,
                                  gboolean     enable_completion);

void xed_history_entry_prepend_text (XedHistoryEntry *entry,
                                     const gchar     *text);
void xed_history_entry_append_text (XedHistoryEntry *entry,
                                    const gchar     *text);

void xed_history_entry_clear (XedHistoryEntry *entry);

void xed_history_entry_set_history_length (XedHistoryEntry *entry,
                                           guint            max_saved);
guint xed_history_entry_get_history_length (XedHistoryEntry *gentry);

void xed_history_entry_set_enable_completion (XedHistoryEntry *entry,
                                              gboolean         enable);
gboolean xed_history_entry_get_enable_completion (XedHistoryEntry *entry);

GtkWidget *xed_history_entry_get_entry (XedHistoryEntry *entry);

G_END_DECLS

#endif /* __XED_HISTORY_ENTRY_H__ */
