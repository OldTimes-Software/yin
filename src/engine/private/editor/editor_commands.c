// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include "engine_private.h"
#include "editor.h"
#include "world.h"

#include "engine_public_game.h"

static void EditorCommand( PL_UNUSED unsigned int argc, PL_UNUSED char **argv )
{
	gameState.mode = ENGINE_MODE_EDITOR;

	Editor_MaterialSelector_Initialize();
}

static void ModelEditorCommand( PL_UNUSED unsigned int argc, PL_UNUSED char **argv )
{
	gameState.mode = ENGINE_MODE_EDITOR;

	Editor_CreateInstance( EDITOR_MODE_MODEL );
}

static void LoadModelCommand( unsigned int argc, char **argv )
{
}

static void ResetGridTransformCommand( unsigned int argc, char **argv )
{
	EditorInstance *editorInstance = Editor_GetCurrentInstance();
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
	EditorInstance *editorInstance = Editor_GetCurrentInstance();
	if ( editorInstance == NULL )
	{
		PRINT_WARNING( "Command failed - no active instance!\n" );
		return;
	}

	if ( editorInstance->mode != EDITOR_MODE_WORLD )
	{
		PRINT_WARNING( "Command failed - invalid active instance mode!\n" );
		return;
	}

	Game_Disconnect();

	editorInstance->worldMode.world = World_Create();
}

static void DestroyWorldCommand( unsigned int argc, char **argv )
{
	EditorInstance *editorInstance = Editor_GetCurrentInstance();
	if ( editorInstance == NULL )
	{
		PRINT_WARNING( "Command failed - no active instance!\n" );
		return;
	}

	if ( editorInstance->mode != EDITOR_MODE_WORLD )
	{
		PRINT_WARNING( "Command failed - invalid active instance mode!\n" );
		return;
	}

	Game_Disconnect();

	World_Destroy( editorInstance->worldMode.world );
	editorInstance->worldMode.world = NULL;
}

static void CreateMeshCommand( unsigned int argc, char **argv )
{
	EditorInstance *editorInstance = Editor_GetCurrentInstance();
	if ( editorInstance == NULL )
	{
		PRINT_WARNING( "Command failed - no active instance!\n" );
		return;
	}

	if ( editorInstance->mode != EDITOR_MODE_WORLD )
	{
		PRINT_WARNING( "Command failed - invalid active instance mode!\n" );
		return;
	}

	World *world = editorInstance->worldMode.world;
	if ( world == NULL )
	{
		return;
	}

	PLVector3 pos = ( PLVector3 ){
	        strtof( argv[ 1 ], NULL ),
	        strtof( argv[ 2 ], NULL ),
	        strtof( argv[ 3 ], NULL ) };

	WorldMesh *mesh = World_Mesh_Create( world );
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
