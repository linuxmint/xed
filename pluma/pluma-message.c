#include "pluma-message.h"
#include "pluma-message-type.h"

#include <string.h>
#include <gobject/gvaluecollector.h>

/**
 * SECTION:pluma-message
 * @short_description: message bus message object
 * @include: pluma/pluma-message.h
 *
 * Communication on a #PlumaMessageBus is done through messages. Messages are
 * sent over the bus and received by connecting callbacks on the message bus.
 * A #PlumaMessage is an instantiation of a #PlumaMessageType, containing
 * values for the arguments as specified in the message type.
 *
 * A message can be seen as a method call, or signal emission depending on
 * who is the sender and who is the receiver. There is no explicit distinction
 * between methods and signals.
 *
 * Since: 2.25.3
 *
 */
#define PLUMA_MESSAGE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), PLUMA_TYPE_MESSAGE, PlumaMessagePrivate))

enum {
	PROP_0,

	PROP_OBJECT_PATH,
	PROP_METHOD,
	PROP_TYPE
};

struct _PlumaMessagePrivate
{
	PlumaMessageType *type;
	gboolean valid;

	GHashTable *values;
};

G_DEFINE_TYPE (PlumaMessage, pluma_message, G_TYPE_OBJECT)

static void
pluma_message_finalize (GObject *object)
{
	PlumaMessage *message = PLUMA_MESSAGE (object);
	
	pluma_message_type_unref (message->priv->type);
	g_hash_table_destroy (message->priv->values);

	G_OBJECT_CLASS (pluma_message_parent_class)->finalize (object);
}

static void
pluma_message_get_property (GObject    *object,
			    guint       prop_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
	PlumaMessage *msg = PLUMA_MESSAGE (object);

	switch (prop_id)
	{
		case PROP_OBJECT_PATH:
			g_value_set_string (value, pluma_message_type_get_object_path (msg->priv->type));
			break;
		case PROP_METHOD:
			g_value_set_string (value, pluma_message_type_get_method (msg->priv->type));
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
pluma_message_set_property (GObject      *object,
			    guint         prop_id,
			    const GValue *value,
			    GParamSpec   *pspec)
{
	PlumaMessage *msg = PLUMA_MESSAGE (object);

	switch (prop_id)
	{
		case PROP_TYPE:
			msg->priv->type = PLUMA_MESSAGE_TYPE (g_value_dup_boxed (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static GValue *
add_value (PlumaMessage *message,
	   const gchar  *key)
{
	GValue *value;
	GType type = pluma_message_type_lookup (message->priv->type, key);
	
	if (type == G_TYPE_INVALID)
		return NULL;
	
	value = g_new0 (GValue, 1);
	g_value_init (value, type);
	g_value_reset (value);

	g_hash_table_insert (message->priv->values, g_strdup (key), value);
	
	return value;
}

static void
pluma_message_class_init (PlumaMessageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	
	object_class->finalize = pluma_message_finalize;
	object_class->get_property = pluma_message_get_property;
	object_class->set_property = pluma_message_set_property;
	
	/**
	 * PlumaMessage:object_path:
	 *
	 * The messages object path (e.g. /pluma/object/path).
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
	 * PlumaMessage:method:
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
	 * PlumaMEssage:type:
	 *
	 * The message type.
	 *
	 */
	g_object_class_install_property (object_class, PROP_TYPE,
					 g_param_spec_boxed ("type",
					 		     "TYPE",
					 		     "The message type",
					 		     PLUMA_TYPE_MESSAGE_TYPE,
					 		     G_PARAM_READWRITE |
					 		     G_PARAM_CONSTRUCT_ONLY |
					 		     G_PARAM_STATIC_STRINGS));

	g_type_class_add_private (object_class, sizeof(PlumaMessagePrivate));
}

static void
destroy_value (GValue *value)
{
	g_value_unset (value);
	g_free (value);
}

static void
pluma_message_init (PlumaMessage *self)
{
	self->priv = PLUMA_MESSAGE_GET_PRIVATE (self);

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
value_lookup (PlumaMessage *message,
	      const gchar  *key,
	      gboolean	    create)
{
	GValue *ret = (GValue *)g_hash_table_lookup (message->priv->values, key);
	
	if (!ret && create)
		ret = add_value (message, key);
	
	return ret;
}

/**
 * pluma_message_get_method:
 * @message: the #PlumaMessage
 *
 * Get the message method.
 *
 * Return value: the message method
 *
 */
const gchar *
pluma_message_get_method (PlumaMessage *message)
{
	g_return_val_if_fail (PLUMA_IS_MESSAGE (message), NULL);
	
	return pluma_message_type_get_method (message->priv->type);
}

/**
 * pluma_message_get_object_path:
 * @message: the #PlumaMessage
 *
 * Get the message object path.
 *
 * Return value: the message object path
 *
 */
const gchar *
pluma_message_get_object_path (PlumaMessage *message)
{
	g_return_val_if_fail (PLUMA_IS_MESSAGE (message), NULL);
	
	return pluma_message_type_get_object_path (message->priv->type);
}

/**
 * pluma_message_set:
 * @message: the #PlumaMessage
 * @...: a NULL terminated variable list of key/value pairs
 *
 * Set values of message arguments. The supplied @var_args should contain
 * pairs of keys and argument values.
 *
 */
void
pluma_message_set (PlumaMessage *message,
		   ...)
{
	va_list ap;

	g_return_if_fail (PLUMA_IS_MESSAGE (message));

	va_start (ap, message);
	pluma_message_set_valist (message, ap);
	va_end (ap);
}

/**
 * pluma_message_set_valist:
 * @message: the #PlumaMessage
 * @var_args: a NULL terminated variable list of key/value pairs
 *
 * Set values of message arguments. The supplied @var_args should contain
 * pairs of keys and argument values.
 *
 */
void
pluma_message_set_valist (PlumaMessage *message,
			  va_list	var_args)
{
	const gchar *key;

	g_return_if_fail (PLUMA_IS_MESSAGE (message));

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
 * pluma_message_set_value:
 * @message: the #PlumaMessage
 * @key: the argument key
 * @value: the argument value
 *
 * Set value of message argument @key to @value.
 *
 */
void
pluma_message_set_value (PlumaMessage *message,
			 const gchar  *key,
			 GValue	      *value)
{
	GValue *container;
	g_return_if_fail (PLUMA_IS_MESSAGE (message));
	
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
 * pluma_message_set_valuesv:
 * @message: the #PlumaMessage
 * @keys: keys to set values for
 * @values: values to set
 * @n_values: number of arguments to set values for
 *
 * Set message argument values.
 *
 */
void
pluma_message_set_valuesv (PlumaMessage	 *message,
			   const gchar	**keys,
			   GValue        *values,
			   gint		  n_values)
{
	gint i;
	
	g_return_if_fail (PLUMA_IS_MESSAGE (message));
	
	for (i = 0; i < n_values; i++)
	{
		pluma_message_set_value (message, keys[i], &values[i]);
	}
}

/**
 * pluma_message_get:
 * @message: the #PlumaMessage
 * @...: a NULL variable argument list of key/value container pairs
 *
 * Get values of message arguments. The supplied @var_args should contain
 * pairs of keys and pointers to variables which are set to the argument
 * value for the specified key.
 *
 */
void 
pluma_message_get (PlumaMessage	*message,
		   ...)
{
	va_list ap;

	g_return_if_fail (PLUMA_IS_MESSAGE (message));
	
	va_start (ap, message);
	pluma_message_get_valist (message, ap);
	va_end (ap);
}

/**
 * pluma_message_get_valist:
 * @message: the #PlumaMessage
 * @var_args: a NULL variable argument list of key/value container pairs
 *
 * Get values of message arguments. The supplied @var_args should contain
 * pairs of keys and pointers to variables which are set to the argument
 * value for the specified key.
 *
 */
void
pluma_message_get_valist (PlumaMessage *message,
			  va_list 	var_args)
{
	const gchar *key;

	g_return_if_fail (PLUMA_IS_MESSAGE (message));
	
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
 * pluma_message_get_value:
 * @message: the #PlumaMessage
 * @key: the argument key
 * @value: value return container
 *
 * Get the value of a specific message argument. @value will be initialized
 * with the correct type.
 *
 */
void 
pluma_message_get_value (PlumaMessage *message,
			 const gchar  *key,
			 GValue	      *value)
{
	GValue *container;
	
	g_return_if_fail (PLUMA_IS_MESSAGE (message));
	
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
 * pluma_message_get_key_type:
 * @message: the #PlumaMessage
 * @key: the argument key
 *
 * Get the type of a message argument.
 *
 * Return value: the type of @key
 *
 */
GType 
pluma_message_get_key_type (PlumaMessage    *message,
			    const gchar	    *key)
{
	g_return_val_if_fail (PLUMA_IS_MESSAGE (message), G_TYPE_INVALID);
	g_return_val_if_fail (message->priv->type != NULL, G_TYPE_INVALID);

	return pluma_message_type_lookup (message->priv->type, key);
}

/**
 * pluma_message_has_key:
 * @message: the #PlumaMessage
 * @key: the argument key
 *
 * Check whether the message has a specific key.
 *
 * Return value: %TRUE if @message has argument @key
 *
 */
gboolean
pluma_message_has_key (PlumaMessage *message,
		       const gchar  *key)
{
	g_return_val_if_fail (PLUMA_IS_MESSAGE (message), FALSE);
	
	return value_lookup (message, key, FALSE) != NULL;
}

typedef struct
{
	PlumaMessage *message;
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
 * pluma_message_validate:
 * @message: the #PlumaMessage
 *
 * Validates the message arguments according to the message type.
 *
 * Return value: %TRUE if the message is valid
 *
 */
gboolean
pluma_message_validate (PlumaMessage *message)
{
	ValidateInfo info = {message, TRUE};

	g_return_val_if_fail (PLUMA_IS_MESSAGE (message), FALSE);
	g_return_val_if_fail (message->priv->type != NULL, FALSE);
	
	if (!message->priv->valid)
	{
		pluma_message_type_foreach (message->priv->type, 
					    (PlumaMessageTypeForeach)validate_key,
					    &info);

		message->priv->valid = info.valid;
	}
	
	return message->priv->valid;
}

// ex:ts=8:noet:
