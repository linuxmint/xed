#ifndef __GEDIT_MESSAGE_BUS_H__
#define __GEDIT_MESSAGE_BUS_H__

#include <glib-object.h>
#include <gedit/gedit-message.h>
#include <gedit/gedit-message-type.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_MESSAGE_BUS			(gedit_message_bus_get_type ())
#define GEDIT_MESSAGE_BUS(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_MESSAGE_BUS, GeditMessageBus))
#define GEDIT_MESSAGE_BUS_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_MESSAGE_BUS, GeditMessageBus const))
#define GEDIT_MESSAGE_BUS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_MESSAGE_BUS, GeditMessageBusClass))
#define GEDIT_IS_MESSAGE_BUS(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_MESSAGE_BUS))
#define GEDIT_IS_MESSAGE_BUS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_MESSAGE_BUS))
#define GEDIT_MESSAGE_BUS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_MESSAGE_BUS, GeditMessageBusClass))

typedef struct _GeditMessageBus		GeditMessageBus;
typedef struct _GeditMessageBusClass	GeditMessageBusClass;
typedef struct _GeditMessageBusPrivate	GeditMessageBusPrivate;

struct _GeditMessageBus {
	GObject parent;
	
	GeditMessageBusPrivate *priv;
};

struct _GeditMessageBusClass {
	GObjectClass parent_class;
	
	void (*dispatch)		(GeditMessageBus  *bus,
					 GeditMessage     *message);
	void (*registered)		(GeditMessageBus  *bus,
					 GeditMessageType *message_type);
	void (*unregistered)		(GeditMessageBus  *bus,
					 GeditMessageType *message_type);
};

typedef void (* GeditMessageCallback) 	(GeditMessageBus *bus,
					 GeditMessage	 *message,
					 gpointer	  userdata);

typedef void (* GeditMessageBusForeach) (GeditMessageType *message_type,
					 gpointer	   userdata);

GType gedit_message_bus_get_type (void) G_GNUC_CONST;

GeditMessageBus *gedit_message_bus_get_default	(void);
GeditMessageBus *gedit_message_bus_new		(void);

/* registering messages */
GeditMessageType *gedit_message_bus_lookup	(GeditMessageBus 	*bus,
						 const gchar		*object_path,
						 const gchar		*method);
GeditMessageType *gedit_message_bus_register	(GeditMessageBus		*bus,
					   	 const gchar 		*object_path,
					  	 const gchar		*method,
					  	 guint		 	 num_optional,
					  	 ...) G_GNUC_NULL_TERMINATED;

void gedit_message_bus_unregister	  (GeditMessageBus	*bus,
					   GeditMessageType	*message_type);

void gedit_message_bus_unregister_all	  (GeditMessageBus	*bus,
					   const gchar		*object_path);

gboolean gedit_message_bus_is_registered  (GeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method);

void gedit_message_bus_foreach		  (GeditMessageBus        *bus,
					   GeditMessageBusForeach  func,
					   gpointer		   userdata);


/* connecting to message events */		   
guint gedit_message_bus_connect	 	  (GeditMessageBus	*bus, 
					   const gchar		*object_path,
					   const gchar		*method,
					   GeditMessageCallback	 callback,
					   gpointer		 userdata,
					   GDestroyNotify        destroy_data);

void gedit_message_bus_disconnect	  (GeditMessageBus	*bus,
					   guint		 id);

void gedit_message_bus_disconnect_by_func (GeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   GeditMessageCallback	 callback,
					   gpointer		 userdata);

/* blocking message event callbacks */
void gedit_message_bus_block		  (GeditMessageBus	*bus,
					   guint		 id);
void gedit_message_bus_block_by_func	  (GeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   GeditMessageCallback	 callback,
					   gpointer		 userdata);

void gedit_message_bus_unblock		  (GeditMessageBus	*bus,
					   guint		 id);
void gedit_message_bus_unblock_by_func	  (GeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   GeditMessageCallback	 callback,
					   gpointer		 userdata);

/* sending messages */
void gedit_message_bus_send_message	  (GeditMessageBus	*bus,
					   GeditMessage		*message);
void gedit_message_bus_send_message_sync  (GeditMessageBus	*bus,
					   GeditMessage		*message);
					  
void gedit_message_bus_send		  (GeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   ...) G_GNUC_NULL_TERMINATED;
GeditMessage *gedit_message_bus_send_sync (GeditMessageBus	*bus,
					   const gchar		*object_path,
					   const gchar		*method,
					   ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS

#endif /* __GEDIT_MESSAGE_BUS_H__ */

// ex:ts=8:noet:
