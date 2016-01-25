#ifndef __XEDIT_MESSAGE_TYPE_H__
#define __XEDIT_MESSAGE_TYPE_H__

#include <glib-object.h>
#include <stdarg.h>

#include "xedit-message.h"

G_BEGIN_DECLS

#define XEDIT_TYPE_MESSAGE_TYPE			(xedit_message_type_get_type ())
#define XEDIT_MESSAGE_TYPE(x)			((XeditMessageType *)(x))

typedef void (*XeditMessageTypeForeach)		(const gchar *key, 
						 GType 	      type, 
						 gboolean     required, 
						 gpointer     user_data);

typedef struct _XeditMessageType			XeditMessageType;

GType xedit_message_type_get_type 		 (void) G_GNUC_CONST;

gboolean xedit_message_type_is_supported 	 (GType type);
gchar *xedit_message_type_identifier		 (const gchar *object_path,
						  const gchar *method);
gboolean xedit_message_type_is_valid_object_path (const gchar *object_path);

XeditMessageType *xedit_message_type_new	 (const gchar *object_path, 
						  const gchar *method,
						  guint	      num_optional,
						  ...) G_GNUC_NULL_TERMINATED;
XeditMessageType *xedit_message_type_new_valist	 (const gchar *object_path,
						  const gchar *method,
						  guint	      num_optional,
						  va_list      va_args);

void xedit_message_type_set			 (XeditMessageType *message_type,
						  guint		   num_optional,
						  ...) G_GNUC_NULL_TERMINATED;
void xedit_message_type_set_valist		 (XeditMessageType *message_type,
						  guint		   num_optional,
						  va_list	           va_args);

XeditMessageType *xedit_message_type_ref 	 (XeditMessageType *message_type);
void xedit_message_type_unref			 (XeditMessageType *message_type);


XeditMessage *xedit_message_type_instantiate_valist (XeditMessageType *message_type,
				       		     va_list	      va_args);
XeditMessage *xedit_message_type_instantiate 	 (XeditMessageType *message_type,
				       		  ...) G_GNUC_NULL_TERMINATED;

const gchar *xedit_message_type_get_object_path	 (XeditMessageType *message_type);
const gchar *xedit_message_type_get_method	 (XeditMessageType *message_type);

GType xedit_message_type_lookup			 (XeditMessageType *message_type,
						  const gchar      *key);
						 
void xedit_message_type_foreach 		 (XeditMessageType 	  *message_type,
						  XeditMessageTypeForeach  func,
						  gpointer	   	   user_data);

G_END_DECLS

#endif /* __XEDIT_MESSAGE_TYPE_H__ */

// ex:ts=8:noet:
