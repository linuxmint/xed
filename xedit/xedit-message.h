#ifndef __XEDIT_MESSAGE_H__
#define __XEDIT_MESSAGE_H__

#include <glib-object.h>
#include <stdarg.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_MESSAGE			(xedit_message_get_type ())
#define XEDIT_MESSAGE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_MESSAGE, XeditMessage))
#define XEDIT_MESSAGE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_MESSAGE, XeditMessage const))
#define XEDIT_MESSAGE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_MESSAGE, XeditMessageClass))
#define XEDIT_IS_MESSAGE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_MESSAGE))
#define XEDIT_IS_MESSAGE_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_MESSAGE))
#define XEDIT_MESSAGE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_MESSAGE, XeditMessageClass))

typedef struct _XeditMessage		XeditMessage;
typedef struct _XeditMessageClass	XeditMessageClass;
typedef struct _XeditMessagePrivate	XeditMessagePrivate;

struct _XeditMessage {
	GObject parent;
	
	XeditMessagePrivate *priv;
};

struct _XeditMessageClass {
	GObjectClass parent_class;
};

GType xedit_message_get_type (void) G_GNUC_CONST;

struct _XeditMessageType xedit_message_get_message_type (XeditMessage *message);

void xedit_message_get			(XeditMessage	 *message,
					 ...) G_GNUC_NULL_TERMINATED;
void xedit_message_get_valist		(XeditMessage	 *message,
					 va_list 	  var_args);
void xedit_message_get_value		(XeditMessage	 *message,
					 const gchar	 *key,
					 GValue		 *value);

void xedit_message_set			(XeditMessage	 *message,
					 ...) G_GNUC_NULL_TERMINATED;
void xedit_message_set_valist		(XeditMessage	 *message,
					 va_list	  	  var_args);
void xedit_message_set_value		(XeditMessage	 *message,
					 const gchar 	 *key,
					 GValue		 *value);
void xedit_message_set_valuesv		(XeditMessage	 *message,
					 const gchar	**keys,
					 GValue		 *values,
					 gint		  n_values);

const gchar *xedit_message_get_object_path (XeditMessage	*message);
const gchar *xedit_message_get_method	(XeditMessage	 *message);

gboolean xedit_message_has_key		(XeditMessage	 *message,
					 const gchar     *key);

GType xedit_message_get_key_type 	(XeditMessage    *message,
			    		 const gchar     *key);

gboolean xedit_message_validate		(XeditMessage	 *message);


G_END_DECLS

#endif /* __XEDIT_MESSAGE_H__ */

// ex:ts=8:noet:
