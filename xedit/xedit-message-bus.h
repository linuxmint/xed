#ifndef __XEDIT_MESSAGE_BUS_H__
#define __XEDIT_MESSAGE_BUS_H__

#include <glib-object.h>
#include <xedit/xedit-message.h>
#include <xedit/xedit-message-type.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_MESSAGE_BUS			(xedit_message_bus_get_type ())
#define XEDIT_MESSAGE_BUS(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_MESSAGE_BUS, XeditMessageBus))
#define XEDIT_MESSAGE_BUS_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_MESSAGE_BUS, XeditMessageBus const))
#define XEDIT_MESSAGE_BUS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_MESSAGE_BUS, XeditMessageBusClass))
#define XEDIT_IS_MESSAGE_BUS(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_MESSAGE_BUS))
#define XEDIT_IS_MESSAGE_BUS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_MESSAGE_BUS))
#define XEDIT_MESSAGE_BUS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_MESSAGE_BUS, XeditMessageBusClass))

typedef struct _XeditMessageBus		XeditMessageBus;
typedef struct _XeditMessageBusClass	XeditMessageBusClass;
typedef struct _XeditMessageBusPrivate	XeditMessageBusPrivate;

struct _XeditMessageBus {
	GObject parent;
	
	XeditMessageBusPrivate *priv;
};

struct _XeditMessageBusClass {
	GObjectClass parent_class;
	
	void (*dispatch)		(XeditMessageBus  *bus,
					 XeditMessage     *message);
	void (*registered)		(XeditMessageBus  *bus,
					 XeditMessageType *message_type);
	void (*unregistered)		(XeditMessageBus  *bus,
					 XeditMessageType *message_type);
};

typedef void (* XeditMessageCallback) 	(XeditMessageBus *bus,
					 XeditMessage	 *message,
					 gpointer	  userdata);

typedef void (* XeditMessageBusForeach) (XeditMessageType *message_type,
					 gpointer	   userdata);

GType xedit_message_bus_get_type (void) G_GNUC_CONST;

XeditMessageBus *xedit_message_bus_get_default	(void);
XeditMessageBus *xedit_message_bus_new		(void);

/* registering messages */
XeditMessageType *xedit_message_bus_lookup	(XeditMessageBus 	*bus,
						 const gchar		*object_path,
						 const gchar		*method);
XeditMessageType *xedit_message_bus_register	(XeditMessageBus		*bus,
					   	 const gchar 		*object_path,
					  	 const gchar		*method,
					  	 guint		 	 num_optional,
					  	 ...) G_GNUC_NULL_TERMINATED;

void xedit_message_bus_unregister	  (XeditMessageBus	*bus,
					   XeditMessageType	*message_type);

void xedit_message_bus_unregister_all	  (XeditMessageBus	*bus,
					   const gchar		*object_path);

gboolean xedit_message_bus_is_registered  (XeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method);

void xedit_message_bus_foreach		  (XeditMessageBus        *bus,
					   XeditMessageBusForeach  func,
					   gpointer		   userdata);


/* connecting to message events */		   
guint xedit_message_bus_connect	 	  (XeditMessageBus	*bus, 
					   const gchar		*object_path,
					   const gchar		*method,
					   XeditMessageCallback	 callback,
					   gpointer		 userdata,
					   GDestroyNotify        destroy_data);

void xedit_message_bus_disconnect	  (XeditMessageBus	*bus,
					   guint		 id);

void xedit_message_bus_disconnect_by_func (XeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   XeditMessageCallback	 callback,
					   gpointer		 userdata);

/* blocking message event callbacks */
void xedit_message_bus_block		  (XeditMessageBus	*bus,
					   guint		 id);
void xedit_message_bus_block_by_func	  (XeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   XeditMessageCallback	 callback,
					   gpointer		 userdata);

void xedit_message_bus_unblock		  (XeditMessageBus	*bus,
					   guint		 id);
void xedit_message_bus_unblock_by_func	  (XeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   XeditMessageCallback	 callback,
					   gpointer		 userdata);

/* sending messages */
void xedit_message_bus_send_message	  (XeditMessageBus	*bus,
					   XeditMessage		*message);
void xedit_message_bus_send_message_sync  (XeditMessageBus	*bus,
					   XeditMessage		*message);
					  
void xedit_message_bus_send		  (XeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   ...) G_GNUC_NULL_TERMINATED;
XeditMessage *xedit_message_bus_send_sync (XeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS

#endif /* __XEDIT_MESSAGE_BUS_H__ */

// ex:ts=8:noet:
