// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>
/* This essentially consolidates the entity component system into one file, whereas
 * before it was all spread through entity.c, which made it really confusing... */

#include <plcore/pl_hashtable.h>

#include "../engine_private.h"

#include "entity.h"

const struct EditorField *YinCore_EntityComponent_GetEditableProperties( const EntityComponent *entityComponent, unsigned int *num )
{
	const EntityComponentBase *base = entityComponent->base;
	assert( base != NULL );
	if ( base == NULL )
		return NULL;

	*num = base->callbackTable->numEditorFields;
	return base->callbackTable->editorFields;
}
