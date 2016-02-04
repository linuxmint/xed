#ifndef __XED_MESSAGE_BUS_H__
#define __XED_MESSAGE_BUS_H__

#include <glib-object.h>
#include <xed/xed-message.h>
#include <xed/xed-message-type.h>

G_BEGIN_DECLS

#define XED_TYPE_MESSAGE_BUS			(xed_message_bus_get_type ())
#define XED_MESSAGE_BUS(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_MESSAGE_BUS, XedMessageBus))
#define XED_MESSAGE_BUS_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_MESSAGE_BUS, XedMessageBus const))
#define XED_MESSAGE_BUS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_MESSAGE_BUS, XedMessageBusClass))
#define XED_IS_MESSAGE_BUS(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_MESSAGE_BUS))
#define XED_IS_MESSAGE_BUS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_MESSAGE_BUS))
#define XED_MESSAGE_BUS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_MESSAGE_BUS, XedMessageBusClass))

typedef struct _XedMessageBus		XedMessageBus;
typedef struct _XedMessageBusClass	XedMessageBusClass;
typedef struct _XedMessageBusPrivate	XedMessageBusPrivate;

struct _XedMessageBus {
	GObject parent;
	
	XedMessageBusPrivate *priv;
};

struct _XedMessageBusClass {
	GObjectClass parent_class;
	
	void (*dispatch)		(XedMessageBus  *bus,
					 XedMessage     *message);
	void (*registered)		(XedMessageBus  *bus,
					 XedMessageType *message_type);
	void (*unregistered)		(XedMessageBus  *bus,
					 XedMessageType *message_type);
};

typedef void (* XedMessageCallback) 	(XedMessageBus *bus,
					 XedMessage	 *message,
					 gpointer	  userdata);

typedef void (* XedMessageBusForeach) (XedMessageType *message_type,
					 gpointer	   userdata);

GType xed_message_bus_get_type (void) G_GNUC_CONST;

XedMessageBus *xed_message_bus_get_default	(void);
XedMessageBus *xed_message_bus_new		(void);

/* registering messages */
XedMessageType *xed_message_bus_lookup	(XedMessageBus 	*bus,
						 const gchar		*object_path,
						 const gchar		*method);
XedMessageType *xed_message_bus_register	(XedMessageBus		*bus,
					   	 const gchar 		*object_path,
					  	 const gchar		*method,
					  	 guint		 	 num_optional,
					  	 ...) G_GNUC_NULL_TERMINATED;

void xed_message_bus_unregister	  (XedMessageBus	*bus,
					   XedMessageType	*message_type);

void xed_message_bus_unregister_all	  (XedMessageBus	*bus,
					   const gchar		*object_path);

gboolean xed_message_bus_is_registered  (XedMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method);

void xed_message_bus_foreach		  (XedMessageBus        *bus,
					   XedMessageBusForeach  func,
					   gpointer		   userdata);


/* connecting to message events */		   
guint xed_message_bus_connect	 	  (XedMessageBus	*bus, 
					   const gchar		*object_path,
					   const gchar		*method,
					   XedMessageCallback	 callback,
					   gpointer		 userdata,
					   GDestroyNotify        destroy_data);

void xed_message_bus_disconnect	  (XedMessageBus	*bus,
					   guint		 id);

void xed_message_bus_disconnect_by_func (XedMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   XedMessageCallback	 callback,
					   gpointer		 userdata);

/* blocking message event callbacks */
void xed_message_bus_block		  (XedMessageBus	*bus,
					   guint		 id);
void xed_message_bus_block_by_func	  (XedMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   XedMessageCallback	 callback,
					   gpointer		 userdata);

void xed_message_bus_unblock		  (XedMessageBus	*bus,
					   guint		 id);
void xed_message_bus_unblock_by_func	  (XedMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   XedMessageCallback	 callback,
					   gpointer		 userdata);

/* sending messages */
void xed_message_bus_send_message	  (XedMessageBus	*bus,
					   XedMessage		*message);
void xed_message_bus_send_message_sync  (XedMessageBus	*bus,
					   XedMessage		*message);
					  
void xed_message_bus_send		  (XedMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   ...) G_GNUC_NULL_TERMINATED;
XedMessage *xed_message_bus_send_sync (XedMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS

#endif /* __XED_MESSAGE_BUS_H__ */

// ex:ts=8:noet:
