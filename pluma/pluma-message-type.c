#include "pluma-message-type.h"

/**
 * SECTION:pluma-message-type
 * @short_description: message type description
 * @include: pluma/pluma-message-type.h
 *
 * A message type is a prototype description for a #PlumaMessage used to
 * transmit messages on a #PlumaMessageBus. The message type describes
 * the Object Path, Method and Arguments of the message.
 *
 * A message type can contain any number of required and optional arguments.
 * To instantiate a #PlumaMessage from a #PlumaMessageType, use 
 * pluma_message_type_instantiate().
 *
 * Registering a new message type on a #PlumaMessageBus with
 * pluma_message_bus_register() internally creates a new #PlumaMessageType. When
 * then using pluma_message_bus_send(), an actual instantiation of the 
 * registered type is internally created and send over the bus.
 *
 * <example>
 * <programlisting>
 * // Defining a new message type
 * PlumaMessageType *message_type = pluma_message_type_new ("/plugins/example",
 *                                                          "method",
 *                                                          0,
 *                                                          "arg1", G_TYPE_STRING,
 *                                                          NULL);
 *
 * // Instantiating an actual message from the type
 * PlumaMessage *message = pluma_message_type_instantiate (message_type,
 *                                                         "arg1", "Hello World",
 *                                                         NULL);
 * </programlisting>
 * </example>
 *
 * Since: 2.25.3
 *
 */
typedef struct
{
	GType type;
	gboolean required;
} ArgumentInfo;

struct _PlumaMessageType
{
	gint ref_count;

	gchar *object_path;
	gchar *method;
	
	guint num_arguments;
	guint num_required;
	
	GHashTable *arguments; // mapping of key -> ArgumentInfo
};

/**
 * pluma_message_type_ref:
 * @message_type: the #PlumaMessageType
 *
 * Increases the reference count on @message_type.
 *
 * Return value: @message_type
 *
 */
PlumaMessageType *
pluma_message_type_ref (PlumaMessageType *message_type)
{
	g_return_val_if_fail (message_type != NULL, NULL);
	g_atomic_int_inc (&message_type->ref_count);
	
	return message_type;
}

/**
 * pluma_message_type_unref:
 * @message_type: the #PlumaMessageType
 *
 * Decreases the reference count on @message_type. When the reference count
 * drops to 0, @message_type is destroyed.
 *
 */
void 
pluma_message_type_unref (PlumaMessageType *message_type)
{
	g_return_if_fail (message_type != NULL);

	if (!g_atomic_int_dec_and_test (&message_type->ref_count))
		return;
	
	g_free (message_type->object_path);
	g_free (message_type->method);
	
	g_hash_table_destroy (message_type->arguments);
	g_free (message_type);
}

/**
 * pluma_message_type_get_type:
 * 
 * Retrieves the GType object which is associated with the
 * #PlumaMessageType class.
 * 
 * Return value: the GType associated with #PlumaMessageType.
 **/
GType 
pluma_message_type_get_type (void)
{
	static GType our_type = 0;

	if (!our_type)
		our_type = g_boxed_type_register_static (
			"PlumaMessageType",
			(GBoxedCopyFunc) pluma_message_type_ref,
			(GBoxedFreeFunc) pluma_message_type_unref);

	return our_type;
}

/**
 * pluma_message_type_identifier:
 * @object_path: the object path
 * @method: the method
 *
 * Get the string identifier for @method at @object_path.
 *
 * Return value: the identifier for @method at @object_path
 *
 */
gchar *
pluma_message_type_identifier (const gchar *object_path,
			       const gchar *method)
{
	return g_strconcat (object_path, ".", method, NULL);
}

/**
 * pluma_message_type_is_valid_object_path:
 * @object_path: the object path
 *
 * Returns whether @object_path is a valid object path
 *
 * Return value: %TRUE if @object_path is a valid object path
 *
 */
gboolean
pluma_message_type_is_valid_object_path (const gchar *object_path)
{
	if (!object_path)
		return FALSE;
	
	/* needs to start with / */
	if (*object_path != '/')
		return FALSE;
	
	while (*object_path)
	{
		if (*object_path == '/')
		{
			++object_path;
			
			if (!*object_path || !(g_ascii_isalpha (*object_path) || *object_path == '_'))
				return FALSE;
		}
		else if (!(g_ascii_isalnum (*object_path) || *object_path == '_'))
		{
			return FALSE;
		}
		
		++object_path;
	}
	
	return TRUE;
}

/**
 * pluma_message_type_is_supported:
 * @type: the #GType
 *
 * Returns if @type is #GType supported by the message system.
 *
 * Return value: %TRUE if @type is a supported #GType
 *
 */
gboolean
pluma_message_type_is_supported (GType type)
{
	gint i = 0;
  
	static const GType type_list[] =
	{
		G_TYPE_BOOLEAN,
		G_TYPE_CHAR,
		G_TYPE_UCHAR,
		G_TYPE_INT,
		G_TYPE_UINT,
		G_TYPE_LONG,
		G_TYPE_ULONG,
		G_TYPE_INT64,
		G_TYPE_UINT64,
		G_TYPE_ENUM,
		G_TYPE_FLAGS,
		G_TYPE_FLOAT,
		G_TYPE_DOUBLE,
		G_TYPE_STRING,
		G_TYPE_POINTER,
		G_TYPE_BOXED,
		G_TYPE_OBJECT,
		G_TYPE_INVALID
	};

	if (!G_TYPE_IS_VALUE_TYPE (type))
		return FALSE;

	while (type_list[i] != G_TYPE_INVALID)
	{
		if (g_type_is_a (type, type_list[i]))
			return TRUE;
		i++;
	}

	return FALSE;
}

/**
 * pluma_message_type_new_valist:
 * @object_path: the object path
 * @method: the method
 * @num_optional: number of optional arguments
 * @var_args: key/gtype pair variable argument list
 *
 * Create a new #PlumaMessageType for @method at @object_path. Argument names
 * and values are supplied by the NULL terminated variable argument list.
 * The last @num_optional provided arguments are considered optional.
 *
 * Return value: the newly constructed #PlumaMessageType
 *
 */
PlumaMessageType *
pluma_message_type_new_valist (const gchar *object_path,
			       const gchar *method,
			       guint        num_optional,
			       va_list      var_args)
{
	PlumaMessageType *message_type;

	g_return_val_if_fail (object_path != NULL, NULL);
	g_return_val_if_fail (method != NULL, NULL);
	g_return_val_if_fail (pluma_message_type_is_valid_object_path (object_path), NULL);

	message_type = g_new0(PlumaMessageType, 1);
	
	message_type->ref_count = 1;
	message_type->object_path = g_strdup(object_path);
	message_type->method = g_strdup(method);
	message_type->num_arguments = 0;
	message_type->arguments = g_hash_table_new_full (g_str_hash,
							 g_str_equal,
							 (GDestroyNotify)g_free,
							 (GDestroyNotify)g_free);

	pluma_message_type_set_valist (message_type, num_optional, var_args);
	return message_type;
}

/**
 * pluma_message_type_new:
 * @object_path: the object path
 * @method: the method
 * @num_optional: number of optional arguments
 * @...: key/gtype pair variable argument list
 *
 * Create a new #PlumaMessageType for @method at @object_path. Argument names
 * and values are supplied by the NULL terminated variable argument list.
 * The last @num_optional provided arguments are considered optional.
 *
 * Return value: the newly constructed #PlumaMessageType
 *
 */
PlumaMessageType *
pluma_message_type_new (const gchar *object_path,
			const gchar *method,
			guint        num_optional,
			...)
{
	PlumaMessageType *message_type;
	va_list var_args;
	
	va_start(var_args, num_optional);
	message_type = pluma_message_type_new_valist (object_path, method, num_optional, var_args);
	va_end(var_args);
	
	return message_type;
}

/**
 * pluma_message_type_set:
 * @message_type: the #PlumaMessageType
 * @num_optional: number of optional arguments
 * @...: key/gtype pair variable argument list
 *
 * Sets argument names/types supplied by the NULL terminated variable
 * argument list. The last @num_optional provided arguments are considered
 * optional.
 *
 */
void
pluma_message_type_set (PlumaMessageType *message_type,
			guint		  num_optional,
			...)
{
	va_list va_args;
	
	va_start (va_args, num_optional);
	pluma_message_type_set_valist (message_type, num_optional, va_args);
	va_end (va_args);
}

/**
 * pluma_message_type_set_valist:
 * @message_type: the #PlumaMessageType
 * @num_optional: number of optional arguments
 * @var_args: key/gtype pair variable argument list
 *
 * Sets argument names/types supplied by the NULL terminated variable
 * argument list @var_args. The last @num_optional provided arguments are 
 * considered optional.
 *
 */
void 
pluma_message_type_set_valist (PlumaMessageType *message_type,
			       guint             num_optional,
			       va_list	         var_args)
{
	const gchar *key;
	ArgumentInfo **optional = g_new0(ArgumentInfo *, num_optional);
	guint i;
	guint added = 0;

	// parse key -> gtype pair arguments
	while ((key = va_arg (var_args, const gchar *)) != NULL)
	{
		// get corresponding GType
		GType gtype = va_arg (var_args, GType);
		ArgumentInfo *info;
		
		if (!pluma_message_type_is_supported (gtype))
		{
			g_error ("Message type '%s' is not supported", g_type_name (gtype));

			pluma_message_type_unref (message_type);
			g_free (optional);
			
			return;
		}
		
		info = g_new(ArgumentInfo, 1);
		info->type = gtype;
		info->required = TRUE;

		g_hash_table_insert (message_type->arguments, g_strdup (key), info);
		
		++message_type->num_arguments;
		++added;
		
		if (num_optional > 0)
		{
			for (i = num_optional - 1; i > 0; --i)
				optional[i] = optional[i - 1];
		
			*optional = info;
		}
	}
	
	message_type->num_required += added;

	// set required for last num_optional arguments
	for (i = 0; i < num_optional; ++i)
	{
		if (optional[i])
		{
			optional[i]->required = FALSE;
			--message_type->num_required;
		}
	}
	
	g_free (optional);
}

/**
 * pluma_message_type_instantiate_valist:
 * @message_type: the #PlumaMessageType
 * @va_args: NULL terminated variable list of key/value pairs
 *
 * Instantiate a new message from the message type with specific values
 * for the message arguments.
 *
 * Return value: the newly created message
 *
 */
PlumaMessage *
pluma_message_type_instantiate_valist (PlumaMessageType *message_type,
				       va_list		 va_args)
{
	PlumaMessage *message;
	
	g_return_val_if_fail (message_type != NULL, NULL);
	
	message = PLUMA_MESSAGE (g_object_new (PLUMA_TYPE_MESSAGE, "type", message_type, NULL));
	pluma_message_set_valist (message, va_args);
	
	return message;
}

/**
 * pluma_message_type_instantiate:
 * @message_type: the #PlumaMessageType
 * @...: NULL terminated variable list of key/value pairs
 *
 * Instantiate a new message from the message type with specific values
 * for the message arguments.
 *
 * Return value: the newly created message
 *
 */
PlumaMessage *
pluma_message_type_instantiate (PlumaMessageType *message_type,
				...)
{
	PlumaMessage *message;
	va_list va_args;
	
	va_start (va_args, message_type);
	message = pluma_message_type_instantiate_valist (message_type, va_args);
	va_end (va_args);
	
	return message;
}

/**
 * pluma_message_type_get_object_path:
 * @message_type: the #PlumaMessageType
 *
 * Get the message type object path.
 *
 * Return value: the message type object path
 *
 */
const gchar *
pluma_message_type_get_object_path (PlumaMessageType *message_type)
{
	return message_type->object_path;
}

/**
 * pluma_message_type_get_method:
 * @message_type: the #PlumaMessageType
 *
 * Get the message type method.
 *
 * Return value: the message type method
 *
 */
const gchar *
pluma_message_type_get_method (PlumaMessageType *message_type)
{
	return message_type->method;
}

/**
 * pluma_message_type_lookup:
 * @message_type: the #PlumaMessageType
 * @key: the argument key
 *
 * Get the argument key #GType.
 *
 * Return value: the #GType of @key
 *
 */
GType
pluma_message_type_lookup (PlumaMessageType *message_type,
			   const gchar      *key)
{
	ArgumentInfo *info = g_hash_table_lookup (message_type->arguments, key);
	
	if (!info)
		return G_TYPE_INVALID;
	
	return info->type;
}

typedef struct
{
	PlumaMessageTypeForeach func;
	gpointer user_data;
} ForeachInfo;

static void
foreach_gtype (const gchar  *key,
	       ArgumentInfo *info,
	       ForeachInfo  *finfo)
{
	finfo->func (key, info->type, info->required, finfo->user_data);
}

/**
 * pluma_message_type_foreach:
 * @message_type: the #PlumaMessageType
 * @func: the callback function
 * @user_data: user data supplied to the callback function
 *
 * Calls @func for each argument in the message type.
 *
 */
void 
pluma_message_type_foreach (PlumaMessageType 	    *message_type,
			    PlumaMessageTypeForeach  func,
			    gpointer		     user_data)
{
	ForeachInfo info = {func, user_data};
	g_hash_table_foreach (message_type->arguments, (GHFunc)foreach_gtype, &info);
}

// ex:ts=8:noet:
