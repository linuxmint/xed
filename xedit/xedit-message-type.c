#include "xedit-message-type.h"

/**
 * SECTION:xedit-message-type
 * @short_description: message type description
 * @include: xedit/xedit-message-type.h
 *
 * A message type is a prototype description for a #XeditMessage used to
 * transmit messages on a #XeditMessageBus. The message type describes
 * the Object Path, Method and Arguments of the message.
 *
 * A message type can contain any number of required and optional arguments.
 * To instantiate a #XeditMessage from a #XeditMessageType, use 
 * xedit_message_type_instantiate().
 *
 * Registering a new message type on a #XeditMessageBus with
 * xedit_message_bus_register() internally creates a new #XeditMessageType. When
 * then using xedit_message_bus_send(), an actual instantiation of the 
 * registered type is internally created and send over the bus.
 *
 * <example>
 * <programlisting>
 * // Defining a new message type
 * XeditMessageType *message_type = xedit_message_type_new ("/plugins/example",
 *                                                          "method",
 *                                                          0,
 *                                                          "arg1", G_TYPE_STRING,
 *                                                          NULL);
 *
 * // Instantiating an actual message from the type
 * XeditMessage *message = xedit_message_type_instantiate (message_type,
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

struct _XeditMessageType
{
	/* FIXME this is an issue for introspection */
	gint ref_count;

	gchar *object_path;
	gchar *method;
	
	guint num_arguments;
	guint num_required;
	
	GHashTable *arguments; // mapping of key -> ArgumentInfo
};

/**
 * xedit_message_type_ref:
 * @message_type: the #XeditMessageType
 *
 * Increases the reference count on @message_type.
 *
 * Return value: @message_type
 *
 */
XeditMessageType *
xedit_message_type_ref (XeditMessageType *message_type)
{
	g_return_val_if_fail (message_type != NULL, NULL);
	g_atomic_int_inc (&message_type->ref_count);
	
	return message_type;
}

/**
 * xedit_message_type_unref:
 * @message_type: the #XeditMessageType
 *
 * Decreases the reference count on @message_type. When the reference count
 * drops to 0, @message_type is destroyed.
 *
 */
void 
xedit_message_type_unref (XeditMessageType *message_type)
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
 * xedit_message_type_get_type:
 * 
 * Retrieves the GType object which is associated with the
 * #XeditMessageType class.
 * 
 * Return value: the GType associated with #XeditMessageType.
 **/
GType 
xedit_message_type_get_type (void)
{
	static GType our_type = 0;

	if (!our_type)
		our_type = g_boxed_type_register_static (
			"XeditMessageType",
			(GBoxedCopyFunc) xedit_message_type_ref,
			(GBoxedFreeFunc) xedit_message_type_unref);

	return our_type;
}

/**
 * xedit_message_type_identifier:
 * @object_path: (allow-none): the object path
 * @method: (allow-none): the method
 *
 * Get the string identifier for @method at @object_path.
 *
 * Return value: the identifier for @method at @object_path
 *
 */
gchar *
xedit_message_type_identifier (const gchar *object_path,
			       const gchar *method)
{
	return g_strconcat (object_path, ".", method, NULL);
}

/**
 * xedit_message_type_is_valid_object_path:
 * @object_path: (allow-none): the object path
 *
 * Returns whether @object_path is a valid object path
 *
 * Return value: %TRUE if @object_path is a valid object path
 *
 */
gboolean
xedit_message_type_is_valid_object_path (const gchar *object_path)
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
 * xedit_message_type_is_supported:
 * @type: the #GType
 *
 * Returns if @type is #GType supported by the message system.
 *
 * Return value: %TRUE if @type is a supported #GType
 *
 */
gboolean
xedit_message_type_is_supported (GType type)
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
 * xedit_message_type_new_valist:
 * @object_path: (allow-none): the object path
 * @method: (allow-none): the method
 * @num_optional: number of optional arguments
 * @var_args: key/gtype pair variable argument list
 *
 * Create a new #XeditMessageType for @method at @object_path. Argument names
 * and values are supplied by the NULL terminated variable argument list.
 * The last @num_optional provided arguments are considered optional.
 *
 * Return value: the newly constructed #XeditMessageType
 *
 */
XeditMessageType *
xedit_message_type_new_valist (const gchar *object_path,
			       const gchar *method,
			       guint        num_optional,
			       va_list      var_args)
{
	XeditMessageType *message_type;

	g_return_val_if_fail (object_path != NULL, NULL);
	g_return_val_if_fail (method != NULL, NULL);
	g_return_val_if_fail (xedit_message_type_is_valid_object_path (object_path), NULL);

	message_type = g_new0(XeditMessageType, 1);
	
	message_type->ref_count = 1;
	message_type->object_path = g_strdup(object_path);
	message_type->method = g_strdup(method);
	message_type->num_arguments = 0;
	message_type->arguments = g_hash_table_new_full (g_str_hash,
							 g_str_equal,
							 (GDestroyNotify)g_free,
							 (GDestroyNotify)g_free);

	xedit_message_type_set_valist (message_type, num_optional, var_args);
	return message_type;
}

/**
 * xedit_message_type_new:
 * @object_path: (allow-none): the object path
 * @method: (allow-none): the method
 * @num_optional: number of optional arguments
 * @...: key/gtype pair variable argument list
 *
 * Create a new #XeditMessageType for @method at @object_path. Argument names
 * and values are supplied by the NULL terminated variable argument list.
 * The last @num_optional provided arguments are considered optional.
 *
 * Return value: the newly constructed #XeditMessageType
 *
 */
XeditMessageType *
xedit_message_type_new (const gchar *object_path,
			const gchar *method,
			guint        num_optional,
			...)
{
	XeditMessageType *message_type;
	va_list var_args;
	
	va_start(var_args, num_optional);
	message_type = xedit_message_type_new_valist (object_path, method, num_optional, var_args);
	va_end(var_args);
	
	return message_type;
}

/**
 * xedit_message_type_set:
 * @message_type: the #XeditMessageType
 * @num_optional: number of optional arguments
 * @...: key/gtype pair variable argument list
 *
 * Sets argument names/types supplied by the NULL terminated variable
 * argument list. The last @num_optional provided arguments are considered
 * optional.
 *
 */
void
xedit_message_type_set (XeditMessageType *message_type,
			guint		  num_optional,
			...)
{
	va_list va_args;
	
	va_start (va_args, num_optional);
	xedit_message_type_set_valist (message_type, num_optional, va_args);
	va_end (va_args);
}

/**
 * xedit_message_type_set_valist:
 * @message_type: the #XeditMessageType
 * @num_optional: number of optional arguments
 * @var_args: key/gtype pair variable argument list
 *
 * Sets argument names/types supplied by the NULL terminated variable
 * argument list @var_args. The last @num_optional provided arguments are 
 * considered optional.
 *
 */
void 
xedit_message_type_set_valist (XeditMessageType *message_type,
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
		
		if (!xedit_message_type_is_supported (gtype))
		{
			g_error ("Message type '%s' is not supported", g_type_name (gtype));

			xedit_message_type_unref (message_type);
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
 * xedit_message_type_instantiate_valist:
 * @message_type: the #XeditMessageType
 * @va_args: NULL terminated variable list of key/value pairs
 *
 * Instantiate a new message from the message type with specific values
 * for the message arguments.
 *
 * Return value: the newly created message
 *
 */
XeditMessage *
xedit_message_type_instantiate_valist (XeditMessageType *message_type,
				       va_list		 va_args)
{
	XeditMessage *message;
	
	g_return_val_if_fail (message_type != NULL, NULL);
	
	message = XEDIT_MESSAGE (g_object_new (XEDIT_TYPE_MESSAGE, "type", message_type, NULL));
	xedit_message_set_valist (message, va_args);
	
	return message;
}

/**
 * xedit_message_type_instantiate:
 * @message_type: the #XeditMessageType
 * @...: NULL terminated variable list of key/value pairs
 *
 * Instantiate a new message from the message type with specific values
 * for the message arguments.
 *
 * Return value: the newly created message
 *
 */
XeditMessage *
xedit_message_type_instantiate (XeditMessageType *message_type,
				...)
{
	XeditMessage *message;
	va_list va_args;
	
	va_start (va_args, message_type);
	message = xedit_message_type_instantiate_valist (message_type, va_args);
	va_end (va_args);
	
	return message;
}

/**
 * xedit_message_type_get_object_path:
 * @message_type: the #XeditMessageType
 *
 * Get the message type object path.
 *
 * Return value: the message type object path
 *
 */
const gchar *
xedit_message_type_get_object_path (XeditMessageType *message_type)
{
	return message_type->object_path;
}

/**
 * xedit_message_type_get_method:
 * @message_type: the #XeditMessageType
 *
 * Get the message type method.
 *
 * Return value: the message type method
 *
 */
const gchar *
xedit_message_type_get_method (XeditMessageType *message_type)
{
	return message_type->method;
}

/**
 * xedit_message_type_lookup:
 * @message_type: the #XeditMessageType
 * @key: the argument key
 *
 * Get the argument key #GType.
 *
 * Return value: the #GType of @key
 *
 */
GType
xedit_message_type_lookup (XeditMessageType *message_type,
			   const gchar      *key)
{
	ArgumentInfo *info = g_hash_table_lookup (message_type->arguments, key);
	
	if (!info)
		return G_TYPE_INVALID;
	
	return info->type;
}

typedef struct
{
	XeditMessageTypeForeach func;
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
 * xedit_message_type_foreach:
 * @message_type: the #XeditMessageType
 * @func: the callback function
 * @user_data: user data supplied to the callback function
 *
 * Calls @func for each argument in the message type.
 *
 */
void 
xedit_message_type_foreach (XeditMessageType 	    *message_type,
			    XeditMessageTypeForeach  func,
			    gpointer		     user_data)
{
	ForeachInfo info = {func, user_data};
	g_hash_table_foreach (message_type->arguments, (GHFunc)foreach_gtype, &info);
}

// ex:ts=8:noet:
