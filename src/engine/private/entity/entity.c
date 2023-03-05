// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_array_vector.h>

#include "engine_private.h"
#include "entity.h"

#define ENT_INTERNAL_TAG 32 /* maximum length of an internal tag */

static unsigned int numTotalSpawns;

/****************************************
 * ENTITY MANAGER
 ****************************************/

/****************************************
 * ENTITY
 ****************************************/

NLNode *YinCore_Entity_Serialize( Entity *self, NLNode *root )
{
	NLNode *entityNode     = NL_PushBackObj( root, "entity" );
	NLNode *componentsNode = NL_PushBackObjArray( entityNode, "components" );

	PLLinkedListNode *node = PlGetFirstNode( self->components );
	while ( node != NULL )
	{
		NLNode *componentNode = NL_PushBackObj( componentsNode, NULL );

		EntityComponent *entityComponent = ( EntityComponent * ) PlGetLinkedListNodeUserData( node );
		NL_PushBackStr( componentNode, "id", entityComponent->base->name );

		const EntityComponentBase *entityComponentTemplate = entityComponent->base;
		if ( entityComponentTemplate->callbackTable->serializeFunction != NULL )
			entityComponentTemplate->callbackTable->serializeFunction( entityComponent, componentNode );

		node = PlGetNextLinkedListNode( node );
	}

	return entityNode;
}

EntityComponent *YinCore_Entity_GetComponentByName( Entity *self, const char *name )
{
	PLLinkedListNode *node = PlGetFirstNode( self->components );
	while ( node != NULL )
	{
		EntityComponent *entityComponent = PlGetLinkedListNodeUserData( node );
		if ( strcmp( entityComponent->base->name, name ) == 0 )
			return entityComponent;

		node = PlGetNextLinkedListNode( node );
	}

	return NULL;
}

EntityComponent *YinCore_Entity_AttachComponentByName( Entity *self, const char *name )
{
	if ( YinCore_Entity_GetComponentByName( self, name ) != NULL )
	{
		PRINT_WARNING( "Entity already has a component of type \"%s\"!\n", name );
		return NULL;
	}

	return YinCore_EntityManager_AddComponentToEntity( self, name );
}

void YinCore_Entity_RemoveComponent( Entity *self, EntityComponent *component )
{
	PlDestroyLinkedListNode( component->listNode );

	const EntityComponentBase *base = component->base;
	if ( base->callbackTable->destroyFunction != NULL )
		base->callbackTable->destroyFunction( component );

	PL_DELETE( component );

	PRINT_DEBUG( "Removed component (%s) from entity (%u)\n", base->name, self->id );
}

void YinCore_Entity_RemoveAllComponents( Entity *self )
{
	PLLinkedListNode *node = PlGetFirstNode( self->components );
	while ( node != NULL )
	{
		EntityComponent *component = PlGetLinkedListNodeUserData( node );
		node                       = PlGetNextLinkedListNode( node );
		YinCore_Entity_RemoveComponent( self, component );
	}
}
