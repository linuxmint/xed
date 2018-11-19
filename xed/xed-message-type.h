#ifndef __XED_MESSAGE_TYPE_H__
#define __XED_MESSAGE_TYPE_H__

#include <glib-object.h>
#include <stdarg.h>

#include "xed-message.h"

G_BEGIN_DECLS

#define XED_TYPE_MESSAGE_TYPE			(xed_message_type_get_type ())
#define XED_MESSAGE_TYPE(x)			((XedMessageType *)(x))

typedef void (*XedMessageTypeForeach)		(const gchar *key,
						 GType 	      type,
						 gboolean     required,
						 gpointer     user_data);

typedef struct _XedMessageType			XedMessageType;

GType xed_message_type_get_type 		 (void) G_GNUC_CONST;

gboolean xed_message_type_is_supported 	 (GType type);
gchar *xed_message_type_identifier		 (const gchar *object_path,
						  const gchar *method);
gboolean xed_message_type_is_valid_object_path (const gchar *object_path);

XedMessageType *xed_message_type_new	 (const gchar *object_path,
						  const gchar *method,
						  guint	      num_optional,
						  ...) G_GNUC_NULL_TERMINATED;
XedMessageType *xed_message_type_new_valist	 (const gchar *object_path,
						  const gchar *method,
						  guint	      num_optional,
						  va_list      var_args);

void xed_message_type_set			 (XedMessageType *message_type,
						  guint		   num_optional,
						  ...) G_GNUC_NULL_TERMINATED;
void xed_message_type_set_valist		 (XedMessageType *message_type,
						  guint		   num_optional,
						  va_list	           var_args);

XedMessageType *xed_message_type_ref 	 (XedMessageType *message_type);
void xed_message_type_unref			 (XedMessageType *message_type);


XedMessage *xed_message_type_instantiate_valist (XedMessageType *message_type,
				       		     va_list	      va_args);
XedMessage *xed_message_type_instantiate 	 (XedMessageType *message_type,
				       		  ...) G_GNUC_NULL_TERMINATED;

const gchar *xed_message_type_get_object_path	 (XedMessageType *message_type);
const gchar *xed_message_type_get_method	 (XedMessageType *message_type);

GType xed_message_type_lookup			 (XedMessageType *message_type,
						  const gchar      *key);

void xed_message_type_foreach 		 (XedMessageType 	  *message_type,
						  XedMessageTypeForeach  func,
						  gpointer	   	   user_data);

G_END_DECLS

#endif /* __XED_MESSAGE_TYPE_H__ */

// ex:ts=8:noet:
