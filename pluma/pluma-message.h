#ifndef __PLUMA_MESSAGE_H__
#define __PLUMA_MESSAGE_H__

#include <glib-object.h>
#include <stdarg.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_MESSAGE			(pluma_message_get_type ())
#define PLUMA_MESSAGE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_MESSAGE, PlumaMessage))
#define PLUMA_MESSAGE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_MESSAGE, PlumaMessage const))
#define PLUMA_MESSAGE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_MESSAGE, PlumaMessageClass))
#define PLUMA_IS_MESSAGE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_MESSAGE))
#define PLUMA_IS_MESSAGE_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_MESSAGE))
#define PLUMA_MESSAGE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_MESSAGE, PlumaMessageClass))

typedef struct _PlumaMessage		PlumaMessage;
typedef struct _PlumaMessageClass	PlumaMessageClass;
typedef struct _PlumaMessagePrivate	PlumaMessagePrivate;

struct _PlumaMessage {
	GObject parent;
	
	PlumaMessagePrivate *priv;
};

struct _PlumaMessageClass {
	GObjectClass parent_class;
};

GType pluma_message_get_type (void) G_GNUC_CONST;

struct _PlumaMessageType pluma_message_get_message_type (PlumaMessage *message);

void pluma_message_get			(PlumaMessage	 *message,
					 ...) G_GNUC_NULL_TERMINATED;
void pluma_message_get_valist		(PlumaMessage	 *message,
					 va_list 	  var_args);
void pluma_message_get_value		(PlumaMessage	 *message,
					 const gchar	 *key,
					 GValue		 *value);

void pluma_message_set			(PlumaMessage	 *message,
					 ...) G_GNUC_NULL_TERMINATED;
void pluma_message_set_valist		(PlumaMessage	 *message,
					 va_list	  	  var_args);
void pluma_message_set_value		(PlumaMessage	 *message,
					 const gchar 	 *key,
					 GValue		 *value);
void pluma_message_set_valuesv		(PlumaMessage	 *message,
					 const gchar	**keys,
					 GValue		 *values,
					 gint		  n_values);

const gchar *pluma_message_get_object_path (PlumaMessage	*message);
const gchar *pluma_message_get_method	(PlumaMessage	 *message);

gboolean pluma_message_has_key		(PlumaMessage	 *message,
					 const gchar     *key);

GType pluma_message_get_key_type 	(PlumaMessage    *message,
			    		 const gchar     *key);

gboolean pluma_message_validate		(PlumaMessage	 *message);


G_END_DECLS

#endif /* __PLUMA_MESSAGE_H__ */

// ex:ts=8:noet:
