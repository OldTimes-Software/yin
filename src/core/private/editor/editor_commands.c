// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include "core_private.h"
#include "editor.h"
#include "world.h"

#include <yin/core_game.h>

static void EditorCommand( PL_UNUSED unsigned int argc, PL_UNUSED char **argv )
{
	Editor_MaterialSelector_Initialize();
}

static void ModelEditorCommand( PL_UNUSED unsigned int argc, PL_UNUSED char **argv )
{
	Editor_CreateInstance( YN_CORE_EDITOR_MODE_MODEL );
}

static void LoadModelCommand( unsigned int argc, char **argv )
{
}

static void ResetGridTransformCommand( unsigned int argc, char **argv )
{
	YNCoreEditorInstance *editorInstance = Editor_GetCurrentInstance();
	if ( editorInstance == NULL )
	{
		PRINT_WARNING( "Command failed - no active instance!\n" );
		return;
	}
	editorInstance->gridTransform = PlMatrix4Identity();
}

//WORLD TOOLS/////////////////////////////////////////////////////////////////////////////////////

static void CreateWorldCommand( unsigned int argc, char **argv )
{
	YNCoreEditorInstance *editorInstance = Editor_GetCurrentInstance();
	if ( editorInstance == NULL )
	{
		PRINT_WARNING( "Command failed - no active instance!\n" );
		return;
	}

	if ( editorInstance->mode != YN_CORE_EDITOR_MODE_WORLD )
	{
		PRINT_WARNING( "Command failed - invalid active instance mode!\n" );
		return;
	}

	Game_Disconnect();

	editorInstance->worldMode.world = YnCore_World_Create();
}

static void DestroyWorldCommand( unsigned int argc, char **argv )
{
	YNCoreEditorInstance *editorInstance = Editor_GetCurrentInstance();
	if ( editorInstance == NULL )
	{
		PRINT_WARNING( "Command failed - no active instance!\n" );
		return;
	}

	if ( editorInstance->mode != YN_CORE_EDITOR_MODE_WORLD )
	{
		PRINT_WARNING( "Command failed - invalid active instance mode!\n" );
		return;
	}

	Game_Disconnect();

	YnCore_World_Destroy( editorInstance->worldMode.world );
	editorInstance->worldMode.world = NULL;
}

static void CreateMeshCommand( unsigned int argc, char **argv )
{
	YNCoreEditorInstance *editorInstance = Editor_GetCurrentInstance();
	if ( editorInstance == NULL )
	{
		PRINT_WARNING( "Command failed - no active instance!\n" );
		return;
	}

	if ( editorInstance->mode != YN_CORE_EDITOR_MODE_WORLD )
	{
		PRINT_WARNING( "Command failed - invalid active instance mode!\n" );
		return;
	}

	YNCoreWorld *world = editorInstance->worldMode.world;
	if ( world == NULL )
		return;

	PLVector3 pos = ( PLVector3 ){
	        strtof( argv[ 1 ], NULL ),
	        strtof( argv[ 2 ], NULL ),
	        strtof( argv[ 3 ], NULL ) };

	YNCoreWorldMesh *mesh = YnCore_WorldMesh_Create( world );
}

void Editor_Commands_Register( void )
{
	PlRegisterConsoleCommand( "editor.clear_instances",
	                          "Destroy all running editor instances.",
	                          0, NULL );
	PlRegisterConsoleCommand( "editor",
	                          "Enable/disable editor mode.",
	                          0, EditorCommand );

	PlRegisterConsoleCommand( "editor.create_world",
	                          "Create a new world instance.",
	                          0, CreateWorldCommand );
	PlRegisterConsoleCommand( "editor.destroy_world",
	                          "Destroy the current world.",
	                          0, DestroyWorldCommand );
	PlRegisterConsoleCommand( "editor.reset_grid_transform",
	                          "Reset the current grid transform.",
	                          0, ResetGridTransformCommand );
	PlRegisterConsoleCommand( "editor.create_mesh",
	                          "Create a new mesh, either at the origin point or given location.",
	                          3, CreateMeshCommand );

	PlRegisterConsoleCommand( "modelEditor",
	                          "Enable/disable model editor.",
	                          0, ModelEditorCommand );
	PlRegisterConsoleCommand( "modelEditor.load_model",
	                          "Load the specified model into the model editor.",
	                          1, LoadModelCommand );
}
