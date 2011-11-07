#ifndef __GEDIT_MESSAGE_TYPE_H__
#define __GEDIT_MESSAGE_TYPE_H__

#include <glib-object.h>
#include <stdarg.h>

#include "gedit-message.h"

G_BEGIN_DECLS

#define GEDIT_TYPE_MESSAGE_TYPE			(gedit_message_type_get_type ())
#define GEDIT_MESSAGE_TYPE(x)			((GeditMessageType *)(x))

typedef void (*GeditMessageTypeForeach)		(const gchar *key, 
						 GType 	      type, 
						 gboolean     required, 
						 gpointer     user_data);

typedef struct _GeditMessageType			GeditMessageType;

GType gedit_message_type_get_type 		 (void) G_GNUC_CONST;

gboolean gedit_message_type_is_supported 	 (GType type);
gchar *gedit_message_type_identifier		 (const gchar *object_path,
						  const gchar *method);
gboolean gedit_message_type_is_valid_object_path (const gchar *object_path);

GeditMessageType *gedit_message_type_new	 (const gchar *object_path, 
						  const gchar *method,
						  guint	      num_optional,
						  ...) G_GNUC_NULL_TERMINATED;
GeditMessageType *gedit_message_type_new_valist	 (const gchar *object_path,
						  const gchar *method,
						  guint	      num_optional,
						  va_list      va_args);

void gedit_message_type_set			 (GeditMessageType *message_type,
						  guint		   num_optional,
						  ...) G_GNUC_NULL_TERMINATED;
void gedit_message_type_set_valist		 (GeditMessageType *message_type,
						  guint		   num_optional,
						  va_list	           va_args);

GeditMessageType *gedit_message_type_ref 	 (GeditMessageType *message_type);
void gedit_message_type_unref			 (GeditMessageType *message_type);


GeditMessage *gedit_message_type_instantiate_valist (GeditMessageType *message_type,
				       		     va_list	      va_args);
GeditMessage *gedit_message_type_instantiate 	 (GeditMessageType *message_type,
				       		  ...) G_GNUC_NULL_TERMINATED;

const gchar *gedit_message_type_get_object_path	 (GeditMessageType *message_type);
const gchar *gedit_message_type_get_method	 (GeditMessageType *message_type);

GType gedit_message_type_lookup			 (GeditMessageType *message_type,
						  const gchar      *key);
						 
void gedit_message_type_foreach 		 (GeditMessageType 	  *message_type,
						  GeditMessageTypeForeach  func,
						  gpointer	   	   user_data);

G_END_DECLS

#endif /* __GEDIT_MESSAGE_TYPE_H__ */

// ex:ts=8:noet:
