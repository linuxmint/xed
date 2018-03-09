#ifndef __XED_DOCUMENT_PRIVATE_H__
#define __XED_DOCUMENT_PRIVATE_H__

#include "xed-document.h"

G_BEGIN_DECLS

glong        _xed_document_get_seconds_since_last_save_or_load  (XedDocument       *doc);

gboolean     _xed_document_needs_saving                         (XedDocument       *doc);

gboolean     _xed_document_get_empty_search                     (XedDocument       *doc);

void         _xed_document_set_create                           (XedDocument       *doc,
                                                                 gboolean           create);

gboolean     _xed_document_get_create                           (XedDocument       *doc);

G_END_DECLS

#endif /* __XED_DOCUMENT_PRIVATE_H__ */
