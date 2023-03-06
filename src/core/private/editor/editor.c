// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <plmodel/plm.h>

#include "core_private.h"
#include "editor.h"

static PLVectorArray  *instances;
static YNCoreEditorInstance *currentInstance = NULL;

void Editor_Initialize( void )
{
	instances = PlCreateVectorArray( 4 );

	Editor_Commands_Register();
}

void YnCore_ShutdownEditor( void )
{
	Editor_MaterialSelector_Shutdown();
}

static void TickEditorInstance( YNCoreEditorInstance *instance )
{
	switch ( instance->mode )
	{
		case YN_CORE_EDITOR_MODE_WORLD:
			break;
		case YN_CORE_EDITOR_MODE_MODEL:
		{
			PLMModel *model = instance->modelMode.model;
			if ( model == NULL )
			{
				break;
			}
			break;
		}
		case YN_CORE_EDITOR_MODE_MATERIAL:
			break;
		default:
			break;
	}
}

void Editor_Tick( void )
{
	unsigned int numInstances = PlGetNumVectorArrayElements( instances );
	for ( unsigned int i = 0; i < numInstances; ++i )
	{
		TickEditorInstance( PlGetVectorArrayElementAt( instances, i ) );
	}
}

static void DrawEditorInstance( YNCoreEditorInstance *instance, const YNCoreViewport *viewport )
{
}

void Editor_Draw( const YNCoreViewport *viewport )
{
	unsigned int numInstances = PlGetNumVectorArrayElements( instances );
	for ( unsigned int i = 0; i < numInstances; ++i )
	{
	}
}

YNCoreEditorInstance *Editor_GetCurrentInstance( void )
{
	return currentInstance;
}

void Editor_SetCurrentInstance( YNCoreEditorInstance *instance )
{
	currentInstance = instance;
}

YNCoreEditorInstance *Editor_CreateInstance( YNCoreEditorMode mode )
{
	YNCoreEditorInstance *instance = PL_NEW( YNCoreEditorInstance );
	instance->mode           = mode;
	instance->gridScale      = 8;
	PlPushBackVectorArrayElement( instances, instance );
	return instance;
}

void Editor_DestroyInstance( YNCoreEditorInstance *instance )
{
}
