// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <plmodel/plm.h>

#include "engine_private.h"
#include "editor.h"

static PLVectorArray  *instances;
static EditorInstance *currentInstance = NULL;

void Editor_Initialize( void )
{
	instances = PlCreateVectorArray( 4 );

	Editor_Commands_Register();
}

void Editor_Shutdown( void )
{
	Editor_MaterialSelector_Shutdown();
}

static void TickEditorInstance( EditorInstance *instance )
{
	switch ( instance->mode )
	{
		case EDITOR_MODE_WORLD:
			break;
		case EDITOR_MODE_MODEL:
		{
			PLMModel *model = instance->modelMode.model;
			if ( model == NULL )
			{
				break;
			}
			break;
		}
		case EDITOR_MODE_MATERIAL:
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

static void DrawEditorInstance( EditorInstance *instance, const YRViewport *viewport )
{
}

void Editor_Draw( const YRViewport *viewport )
{
	unsigned int numInstances = PlGetNumVectorArrayElements( instances );
	for ( unsigned int i = 0; i < numInstances; ++i )
	{
	}
}

EditorInstance *Editor_GetCurrentInstance( void )
{
	return currentInstance;
}

void Editor_SetCurrentInstance( EditorInstance *instance )
{
	currentInstance = instance;
}

EditorInstance *Editor_CreateInstance( EditorMode mode )
{
	EditorInstance *instance = PL_NEW( EditorInstance );
	instance->mode           = mode;
	instance->gridScale      = 8;
	PlPushBackVectorArrayElement( instances, instance );
	return instance;
}

void Editor_DestroyInstance( EditorInstance *instance )
{
}
