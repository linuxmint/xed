#ifndef __PLUMA_MESSAGE_BUS_H__
#define __PLUMA_MESSAGE_BUS_H__

#include <glib-object.h>
#include <pluma/pluma-message.h>
#include <pluma/pluma-message-type.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_MESSAGE_BUS			(pluma_message_bus_get_type ())
#define PLUMA_MESSAGE_BUS(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_MESSAGE_BUS, PlumaMessageBus))
#define PLUMA_MESSAGE_BUS_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_MESSAGE_BUS, PlumaMessageBus const))
#define PLUMA_MESSAGE_BUS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_MESSAGE_BUS, PlumaMessageBusClass))
#define PLUMA_IS_MESSAGE_BUS(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_MESSAGE_BUS))
#define PLUMA_IS_MESSAGE_BUS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_MESSAGE_BUS))
#define PLUMA_MESSAGE_BUS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_MESSAGE_BUS, PlumaMessageBusClass))

typedef struct _PlumaMessageBus		PlumaMessageBus;
typedef struct _PlumaMessageBusClass	PlumaMessageBusClass;
typedef struct _PlumaMessageBusPrivate	PlumaMessageBusPrivate;

struct _PlumaMessageBus {
	GObject parent;
	
	PlumaMessageBusPrivate *priv;
};

struct _PlumaMessageBusClass {
	GObjectClass parent_class;
	
	void (*dispatch)		(PlumaMessageBus  *bus,
					 PlumaMessage     *message);
	void (*registered)		(PlumaMessageBus  *bus,
					 PlumaMessageType *message_type);
	void (*unregistered)		(PlumaMessageBus  *bus,
					 PlumaMessageType *message_type);
};

typedef void (* PlumaMessageCallback) 	(PlumaMessageBus *bus,
					 PlumaMessage	 *message,
					 gpointer	  userdata);

typedef void (* PlumaMessageBusForeach) (PlumaMessageType *message_type,
					 gpointer	   userdata);

GType pluma_message_bus_get_type (void) G_GNUC_CONST;

PlumaMessageBus *pluma_message_bus_get_default	(void);
PlumaMessageBus *pluma_message_bus_new		(void);

/* registering messages */
PlumaMessageType *pluma_message_bus_lookup	(PlumaMessageBus 	*bus,
						 const gchar		*object_path,
						 const gchar		*method);
PlumaMessageType *pluma_message_bus_register	(PlumaMessageBus		*bus,
					   	 const gchar 		*object_path,
					  	 const gchar		*method,
					  	 guint		 	 num_optional,
					  	 ...) G_GNUC_NULL_TERMINATED;

void pluma_message_bus_unregister	  (PlumaMessageBus	*bus,
					   PlumaMessageType	*message_type);

void pluma_message_bus_unregister_all	  (PlumaMessageBus	*bus,
					   const gchar		*object_path);

gboolean pluma_message_bus_is_registered  (PlumaMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method);

void pluma_message_bus_foreach		  (PlumaMessageBus        *bus,
					   PlumaMessageBusForeach  func,
					   gpointer		   userdata);


/* connecting to message events */		   
guint pluma_message_bus_connect	 	  (PlumaMessageBus	*bus, 
					   const gchar		*object_path,
					   const gchar		*method,
					   PlumaMessageCallback	 callback,
					   gpointer		 userdata,
					   GDestroyNotify        destroy_data);

void pluma_message_bus_disconnect	  (PlumaMessageBus	*bus,
					   guint		 id);

void pluma_message_bus_disconnect_by_func (PlumaMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   PlumaMessageCallback	 callback,
					   gpointer		 userdata);

/* blocking message event callbacks */
void pluma_message_bus_block		  (PlumaMessageBus	*bus,
					   guint		 id);
void pluma_message_bus_block_by_func	  (PlumaMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   PlumaMessageCallback	 callback,
					   gpointer		 userdata);

void pluma_message_bus_unblock		  (PlumaMessageBus	*bus,
					   guint		 id);
void pluma_message_bus_unblock_by_func	  (PlumaMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   PlumaMessageCallback	 callback,
					   gpointer		 userdata);

/* sending messages */
void pluma_message_bus_send_message	  (PlumaMessageBus	*bus,
					   PlumaMessage		*message);
void pluma_message_bus_send_message_sync  (PlumaMessageBus	*bus,
					   PlumaMessage		*message);
					  
void pluma_message_bus_send		  (PlumaMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   ...) G_GNUC_NULL_TERMINATED;
PlumaMessage *pluma_message_bus_send_sync (PlumaMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS

#endif /* __PLUMA_MESSAGE_BUS_H__ */

// ex:ts=8:noet:
