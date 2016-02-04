#ifndef __XED_MESSAGE_H__
#define __XED_MESSAGE_H__

#include <glib-object.h>
#include <stdarg.h>

G_BEGIN_DECLS

#define XED_TYPE_MESSAGE			(xed_message_get_type ())
#define XED_MESSAGE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_MESSAGE, XedMessage))
#define XED_MESSAGE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_MESSAGE, XedMessage const))
#define XED_MESSAGE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_MESSAGE, XedMessageClass))
#define XED_IS_MESSAGE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_MESSAGE))
#define XED_IS_MESSAGE_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_MESSAGE))
#define XED_MESSAGE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_MESSAGE, XedMessageClass))

typedef struct _XedMessage		XedMessage;
typedef struct _XedMessageClass	XedMessageClass;
typedef struct _XedMessagePrivate	XedMessagePrivate;

struct _XedMessage {
	GObject parent;
	
	XedMessagePrivate *priv;
};

struct _XedMessageClass {
	GObjectClass parent_class;
};

GType xed_message_get_type (void) G_GNUC_CONST;

struct _XedMessageType xed_message_get_message_type (XedMessage *message);

void xed_message_get			(XedMessage	 *message,
					 ...) G_GNUC_NULL_TERMINATED;
void xed_message_get_valist		(XedMessage	 *message,
					 va_list 	  var_args);
void xed_message_get_value		(XedMessage	 *message,
					 const gchar	 *key,
					 GValue		 *value);

void xed_message_set			(XedMessage	 *message,
					 ...) G_GNUC_NULL_TERMINATED;
void xed_message_set_valist		(XedMessage	 *message,
					 va_list	  	  var_args);
void xed_message_set_value		(XedMessage	 *message,
					 const gchar 	 *key,
					 GValue		 *value);
void xed_message_set_valuesv		(XedMessage	 *message,
					 const gchar	**keys,
					 GValue		 *values,
					 gint		  n_values);

const gchar *xed_message_get_object_path (XedMessage	*message);
const gchar *xed_message_get_method	(XedMessage	 *message);

gboolean xed_message_has_key		(XedMessage	 *message,
					 const gchar     *key);

GType xed_message_get_key_type 	(XedMessage    *message,
			    		 const gchar     *key);

gboolean xed_message_validate		(XedMessage	 *message);


G_END_DECLS

#endif /* __XED_MESSAGE_H__ */

// ex:ts=8:noet:
