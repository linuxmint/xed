#include "gedit-message.h"
#include "gedit-message-type.h"

#include <string.h>
#include <gobject/gvaluecollector.h>

/**
 * SECTION:gedit-message
 * @short_description: message bus message object
 * @include: gedit/gedit-message.h
 *
 * Communication on a #GeditMessageBus is done through messages. Messages are
 * sent over the bus and received by connecting callbacks on the message bus.
 * A #GeditMessage is an instantiation of a #GeditMessageType, containing
 * values for the arguments as specified in the message type.
 *
 * A message can be seen as a method call, or signal emission depending on
 * who is the sender and who is the receiver. There is no explicit distinction
 * between methods and signals.
 *
 * Since: 2.25.3
 *
 */
#define GEDIT_MESSAGE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_TYPE_MESSAGE, GeditMessagePrivate))

enum {
	PROP_0,

	PROP_OBJECT_PATH,
	PROP_METHOD,
	PROP_TYPE
};

struct _GeditMessagePrivate
{
	GeditMessageType *type;
	gboolean valid;

	GHashTable *values;
};

G_DEFINE_TYPE (GeditMessage, gedit_message, G_TYPE_OBJECT)

static void
gedit_message_finalize (GObject *object)
{
	GeditMessage *message = GEDIT_MESSAGE (object);
	
	gedit_message_type_unref (message->priv->type);
	g_hash_table_destroy (message->priv->values);

	G_OBJECT_CLASS (gedit_message_parent_class)->finalize (object);
}

static void
gedit_message_get_property (GObject    *object,
			    guint       prop_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
	GeditMessage *msg = GEDIT_MESSAGE (object);

	switch (prop_id)
	{
		case PROP_OBJECT_PATH:
			g_value_set_string (value, gedit_message_type_get_object_path (msg->priv->type));
			break;
		case PROP_METHOD:
			g_value_set_string (value, gedit_message_type_get_method (msg->priv->type));
			break;
		case PROP_TYPE:
			g_value_set_boxed (value, msg->priv->type);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_message_set_property (GObject      *object,
			    guint         prop_id,
			    const GValue *value,
			    GParamSpec   *pspec)
{
	GeditMessage *msg = GEDIT_MESSAGE (object);

	switch (prop_id)
	{
		case PROP_TYPE:
			msg->priv->type = GEDIT_MESSAGE_TYPE (g_value_dup_boxed (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static GValue *
add_value (GeditMessage *message,
	   const gchar  *key)
{
	GValue *value;
	GType type = gedit_message_type_lookup (message->priv->type, key);
	
	if (type == G_TYPE_INVALID)
		return NULL;
	
	value = g_new0 (GValue, 1);
	g_value_init (value, type);
	g_value_reset (value);

	g_hash_table_insert (message->priv->values, g_strdup (key), value);
	
	return value;
}

static void
gedit_message_class_init (GeditMessageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	
	object_class->finalize = gedit_message_finalize;
	object_class->get_property = gedit_message_get_property;
	object_class->set_property = gedit_message_set_property;
	
	/**
	 * GeditMessage:object_path:
	 *
	 * The messages object path (e.g. /gedit/object/path).
	 *
	 */
	g_object_class_install_property (object_class, PROP_OBJECT_PATH,
					 g_param_spec_string ("object-path",
							      "OBJECT_PATH",
							      "The message object path",
							      NULL,
							      G_PARAM_READABLE |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * GeditMessage:method:
	 *
	 * The messages method.
	 *
	 */
	g_object_class_install_property (object_class, PROP_METHOD,
					 g_param_spec_string ("method",
							      "METHOD",
							      "The message method",
							      NULL,
							      G_PARAM_READABLE |
							      G_PARAM_STATIC_STRINGS));
	
	/**
	 * GeditMEssage:type:
	 *
	 * The message type.
	 *
	 */
	g_object_class_install_property (object_class, PROP_TYPE,
					 g_param_spec_boxed ("type",
					 		     "TYPE",
					 		     "The message type",
					 		     GEDIT_TYPE_MESSAGE_TYPE,
					 		     G_PARAM_READWRITE |
					 		     G_PARAM_CONSTRUCT_ONLY |
					 		     G_PARAM_STATIC_STRINGS));

	g_type_class_add_private (object_class, sizeof(GeditMessagePrivate));
}

static void
destroy_value (GValue *value)
{
	g_value_unset (value);
	g_free (value);
}

static void
gedit_message_init (GeditMessage *self)
{
	self->priv = GEDIT_MESSAGE_GET_PRIVATE (self);

	self->priv->values = g_hash_table_new_full (g_str_hash,
						    g_str_equal,
						    (GDestroyNotify)g_free,
						    (GDestroyNotify)destroy_value);
}

static gboolean
set_value_real (GValue 	     *to, 
		const GValue *from)
{
	GType from_type;
	GType to_type;
	
	from_type = G_VALUE_TYPE (from);
	to_type = G_VALUE_TYPE (to);

	if (!g_type_is_a (from_type, to_type))
	{		
		if (!g_value_transform (from, to))
		{
			g_warning ("%s: Unable to make conversion from %s to %s",
				   G_STRLOC,
				   g_type_name (from_type),
				   g_type_name (to_type));
			return FALSE;
		}
		
		return TRUE;
	}
	
	g_value_copy (from, to);
	return TRUE;
}

inline static GValue *
value_lookup (GeditMessage *message,
	      const gchar  *key,
	      gboolean	    create)
{
	GValue *ret = (GValue *)g_hash_table_lookup (message->priv->values, key);
	
	if (!ret && create)
		ret = add_value (message, key);
	
	return ret;
}

/**
 * gedit_message_get_method:
 * @message: the #GeditMessage
 *
 * Get the message method.
 *
 * Return value: the message method
 *
 */
const gchar *
gedit_message_get_method (GeditMessage *message)
{
	g_return_val_if_fail (GEDIT_IS_MESSAGE (message), NULL);
	
	return gedit_message_type_get_method (message->priv->type);
}

/**
 * gedit_message_get_object_path:
 * @message: the #GeditMessage
 *
 * Get the message object path.
 *
 * Return value: the message object path
 *
 */
const gchar *
gedit_message_get_object_path (GeditMessage *message)
{
	g_return_val_if_fail (GEDIT_IS_MESSAGE (message), NULL);
	
	return gedit_message_type_get_object_path (message->priv->type);
}

/**
 * gedit_message_set:
 * @message: the #GeditMessage
 * @...: a NULL terminated variable list of key/value pairs
 *
 * Set values of message arguments. The supplied @var_args should contain
 * pairs of keys and argument values.
 *
 */
void
gedit_message_set (GeditMessage *message,
		   ...)
{
	va_list ap;

	g_return_if_fail (GEDIT_IS_MESSAGE (message));

	va_start (ap, message);
	gedit_message_set_valist (message, ap);
	va_end (ap);
}

/**
 * gedit_message_set_valist:
 * @message: the #GeditMessage
 * @var_args: a NULL terminated variable list of key/value pairs
 *
 * Set values of message arguments. The supplied @var_args should contain
 * pairs of keys and argument values.
 *
 */
void
gedit_message_set_valist (GeditMessage *message,
			  va_list	var_args)
{
	const gchar *key;

	g_return_if_fail (GEDIT_IS_MESSAGE (message));

	while ((key = va_arg (var_args, const gchar *)) != NULL)
	{
		/* lookup the key */
		GValue *container = value_lookup (message, key, TRUE);
		GValue value = {0,};
		gchar *error = NULL;
		
		if (!container)
		{
			g_warning ("%s: Cannot set value for %s, does not exist", 
				   G_STRLOC,
				   key);
			
			/* skip value */
			va_arg (var_args, gpointer);
			continue;
		}
		
		g_value_init (&value, G_VALUE_TYPE (container));
		G_VALUE_COLLECT (&value, var_args, 0, &error);
		
		if (error)
		{
			g_warning ("%s: %s", G_STRLOC, error);
			continue;
		}

		set_value_real (container, &value);
		g_value_unset (&value);
	}
}

/**
 * gedit_message_set_value:
 * @message: the #GeditMessage
 * @key: the argument key
 * @value: the argument value
 *
 * Set value of message argument @key to @value.
 *
 */
void
gedit_message_set_value (GeditMessage *message,
			 const gchar  *key,
			 GValue	      *value)
{
	GValue *container;
	g_return_if_fail (GEDIT_IS_MESSAGE (message));
	
	container = value_lookup (message, key, TRUE);
	
	if (!container)
	{
		g_warning ("%s: Cannot set value for %s, does not exist", 
			   G_STRLOC, 
			   key);
		return;
	}
	
	set_value_real (container, value);
}

/**
 * gedit_message_set_valuesv:
 * @message: the #GeditMessage
 * @keys: keys to set values for
 * @values: values to set
 * @n_values: number of arguments to set values for
 *
 * Set message argument values.
 *
 */
void
gedit_message_set_valuesv (GeditMessage	 *message,
			   const gchar	**keys,
			   GValue        *values,
			   gint		  n_values)
{
	gint i;
	
	g_return_if_fail (GEDIT_IS_MESSAGE (message));
	
	for (i = 0; i < n_values; i++)
	{
		gedit_message_set_value (message, keys[i], &values[i]);
	}
}

/**
 * gedit_message_get:
 * @message: the #GeditMessage
 * @...: a NULL variable argument list of key/value container pairs
 *
 * Get values of message arguments. The supplied @var_args should contain
 * pairs of keys and pointers to variables which are set to the argument
 * value for the specified key.
 *
 */
void 
gedit_message_get (GeditMessage	*message,
		   ...)
{
	va_list ap;

	g_return_if_fail (GEDIT_IS_MESSAGE (message));
	
	va_start (ap, message);
	gedit_message_get_valist (message, ap);
	va_end (ap);
}

/**
 * gedit_message_get_valist:
 * @message: the #GeditMessage
 * @var_args: a NULL variable argument list of key/value container pairs
 *
 * Get values of message arguments. The supplied @var_args should contain
 * pairs of keys and pointers to variables which are set to the argument
 * value for the specified key.
 *
 */
void
gedit_message_get_valist (GeditMessage *message,
			  va_list 	var_args)
{
	const gchar *key;

	g_return_if_fail (GEDIT_IS_MESSAGE (message));
	
	while ((key = va_arg (var_args, const gchar *)) != NULL)
	{
		GValue *container;
		GValue copy = {0,};
		gchar *error = NULL;

		container = value_lookup (message, key, FALSE);
	
		if (!container)
		{		
			/* skip value */
			va_arg (var_args, gpointer);
			continue;
		}
		
		/* copy the value here, to be sure it isn't tainted */
		g_value_init (&copy, G_VALUE_TYPE (container));
		g_value_copy (container, &copy);
		
		G_VALUE_LCOPY (&copy, var_args, 0, &error);
		
		if (error)
		{
			g_warning ("%s: %s", G_STRLOC, error);
			g_free (error);
			
			/* purposely leak the value here, because it might
			   be in a bad state */
			continue;
		}
		
		g_value_unset (&copy);
	}
}

/**
 * gedit_message_get_value:
 * @message: the #GeditMessage
 * @key: the argument key
 * @value: value return container
 *
 * Get the value of a specific message argument. @value will be initialized
 * with the correct type.
 *
 */
void 
gedit_message_get_value (GeditMessage *message,
			 const gchar  *key,
			 GValue	      *value)
{
	GValue *container;
	
	g_return_if_fail (GEDIT_IS_MESSAGE (message));
	
	container = value_lookup (message, key, FALSE);
	
	if (!container)
	{
		g_warning ("%s: Invalid key `%s'",
			   G_STRLOC,
			   key);
		return;
	}
	
	g_value_init (value, G_VALUE_TYPE (container));
	set_value_real (value, container);
}

/**
 * gedit_message_get_key_type:
 * @message: the #GeditMessage
 * @key: the argument key
 *
 * Get the type of a message argument.
 *
 * Return value: the type of @key
 *
 */
GType 
gedit_message_get_key_type (GeditMessage    *message,
			    const gchar	    *key)
{
	g_return_val_if_fail (GEDIT_IS_MESSAGE (message), G_TYPE_INVALID);
	g_return_val_if_fail (message->priv->type != NULL, G_TYPE_INVALID);

	return gedit_message_type_lookup (message->priv->type, key);
}

/**
 * gedit_message_has_key:
 * @message: the #GeditMessage
 * @key: the argument key
 *
 * Check whether the message has a specific key.
 *
 * Return value: %TRUE if @message has argument @key
 *
 */
gboolean
gedit_message_has_key (GeditMessage *message,
		       const gchar  *key)
{
	g_return_val_if_fail (GEDIT_IS_MESSAGE (message), FALSE);
	
	return value_lookup (message, key, FALSE) != NULL;
}

typedef struct
{
	GeditMessage *message;
	gboolean valid;	
} ValidateInfo;

static void
validate_key (const gchar  *key,
	      GType         type,
	      gboolean	    required,
	      ValidateInfo *info)
{
	GValue *value;
	
	if (!info->valid || !required)
		return;
	
	value = value_lookup (info->message, key, FALSE);
	
	if (!value)
		info->valid = FALSE;
}

/**
 * gedit_message_validate:
 * @message: the #GeditMessage
 *
 * Validates the message arguments according to the message type.
 *
 * Return value: %TRUE if the message is valid
 *
 */
gboolean
gedit_message_validate (GeditMessage *message)
{
	ValidateInfo info = {message, TRUE};

	g_return_val_if_fail (GEDIT_IS_MESSAGE (message), FALSE);
	g_return_val_if_fail (message->priv->type != NULL, FALSE);
	
	if (!message->priv->valid)
	{
		gedit_message_type_foreach (message->priv->type, 
					    (GeditMessageTypeForeach)validate_key,
					    &info);

		message->priv->valid = info.valid;
	}
	
	return message->priv->valid;
}

// ex:ts=8:noet:
