// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "core_private.h"
#include "renderer.h"

#include "node/public/node.h"

/**********************************************************/
/** Shaders **/

static PLLinkedList *shaderPrograms;
PLGShaderProgram	 *defaultShaderPrograms[ RS_MAX_DEFAULT_SHADERS ];

static void RS_RegisterShaderStage( PLGShaderProgram *program, PLGShaderStageType type, const char *path, char definitions[][ PLG_MAX_DEFINITION_LENGTH ], unsigned int numDefinitions )
{
	PLFile *filePtr = PlOpenFile( path, true );
	if ( filePtr == NULL )
		PRINT_ERROR( "Failed to find shader \"%s\"!\nPL: %s\n", path, PlGetError() );

	PLGShaderStage *stage = PlgCreateShaderStage( type );
	PlgSetShaderStageDefinitions( stage, definitions, numDefinitions );

	size_t length = PlGetFileSize( filePtr );
	char	 *buffer = PlMAllocA( length + 1 );
	PlReadFile( filePtr, buffer, length, 1 );
	buffer[ length ] = '\0';

	PlCloseFile( filePtr );

	PlgCompileShaderStage( stage, buffer, length );
	if ( PlGetFunctionResult() != PL_RESULT_SUCCESS )
		PRINT_ERROR( "Failed to register stage, \"%s\"!\nPL: %s\n", path, PlGetError() );

	PlgAttachShaderStage( program, stage );

	PlFree( buffer );
}

static YNCoreShaderProgramIndex *RS_ParseShaderProgram( NLNode *root )
{
	YNCoreShaderProgramIndex program;
	memset( &program, 0, sizeof( YNCoreShaderProgramIndex ) );

	const char *vertexPath = NL_GetStrByName( root, "vertexPath", NULL );
	const char *fragmentPath = NL_GetStrByName( root, "fragmentPath", NULL );

	if ( vertexPath == NULL || fragmentPath == NULL )
	{
		PRINT_WARNING( "No vertex/fragment stage defined in program!\n" );
		return NULL;
	}

	program.internalPtr = PlgCreateShaderProgram();
	if ( program.internalPtr == NULL )
	{
		PRINT_WARNING( "Failed to create shader program!\nPL: %s\n", PlGetError() );
		return NULL;
	}

	snprintf( program.shaderPaths[ PLG_SHADER_TYPE_VERTEX ], PL_SYSTEM_MAX_PATH, "%s", vertexPath );
	snprintf( program.shaderPaths[ PLG_SHADER_TYPE_FRAGMENT ], PL_SYSTEM_MAX_PATH, "%s", fragmentPath );

	const char *internalName = NL_GetStrByName( root, "description", NULL );
	if ( internalName != NULL )
		snprintf( program.internalName, sizeof( program.internalName ), "%s", internalName );
	else
	{
		PRINT_WARNING( "Shader program with no internal name provided!\n" );
		snprintf( program.internalName, sizeof( program.internalName ), "unnamed" );
	}

	/* these allow for the program to specify what
	 * definitions should be set prior to compiling
	 * the given shader. */

	char fragmentDefinitions[ PLG_MAX_DEFINITIONS ][ PLG_MAX_DEFINITION_LENGTH ];
	PL_ZERO( fragmentDefinitions, PLG_MAX_DEFINITION_LENGTH * PLG_MAX_DEFINITIONS );

	char vertexDefinitions[ PLG_MAX_DEFINITIONS ][ PLG_MAX_DEFINITION_LENGTH ];
	PL_ZERO( vertexDefinitions, PLG_MAX_DEFINITION_LENGTH * PLG_MAX_DEFINITIONS );

	unsigned int numDefinitions[ PLG_MAX_SHADER_TYPES ];
	PL_ZERO( numDefinitions, sizeof( unsigned int ) * PLG_MAX_SHADER_TYPES );

	NLNode *child = NL_GetChildByName( root, "definitions" );
	if ( child != NULL )
	{
		NLNode *subChild;
		if ( ( subChild = NL_GetChildByName( child, "fragment" ) ) != NULL )
		{
			numDefinitions[ PLG_SHADER_TYPE_FRAGMENT ] = NL_GetNumOfChildren( subChild );
			if ( numDefinitions[ PLG_SHADER_TYPE_FRAGMENT ] > PLG_MAX_DEFINITIONS )
				numDefinitions[ PLG_SHADER_TYPE_FRAGMENT ] = PLG_MAX_DEFINITIONS;

			subChild = NL_GetFirstChild( subChild );
			for ( unsigned int i = 0; i < numDefinitions[ PLG_SHADER_TYPE_FRAGMENT ]; ++i )
			{
				if ( subChild == NULL )
				{
					PRINT_WARNING( "Hit an invalid child, aborting early!\n" );
					numDefinitions[ PLG_SHADER_TYPE_FRAGMENT ] = i;
					break;
				}

				NL_GetStr( subChild, fragmentDefinitions[ i ], PLG_MAX_DEFINITION_LENGTH );
				subChild = NL_GetNextChild( subChild );
			}
		}
		if ( ( subChild = NL_GetChildByName( child, "vertex" ) ) != NULL )
		{
			numDefinitions[ PLG_SHADER_TYPE_VERTEX ] = NL_GetNumOfChildren( subChild );
			if ( numDefinitions[ PLG_SHADER_TYPE_VERTEX ] > PLG_MAX_DEFINITIONS )
				numDefinitions[ PLG_SHADER_TYPE_VERTEX ] = PLG_MAX_DEFINITIONS;

			subChild = NL_GetFirstChild( subChild );
			for ( unsigned int i = 0; i < numDefinitions[ PLG_SHADER_TYPE_VERTEX ]; ++i )
			{
				if ( subChild == NULL )
				{
					PRINT_WARNING( "Hit an invalid child, aborting early!\n" );
					numDefinitions[ PLG_SHADER_TYPE_FRAGMENT ] = i;
					break;
				}

				NL_GetStr( subChild, vertexDefinitions[ i ], PLG_MAX_DEFINITION_LENGTH );
				subChild = NL_GetNextChild( subChild );
			}
		}
	}

	RS_RegisterShaderStage( program.internalPtr, PLG_SHADER_TYPE_VERTEX, vertexPath, vertexDefinitions, numDefinitions[ PLG_SHADER_TYPE_VERTEX ] );
	RS_RegisterShaderStage( program.internalPtr, PLG_SHADER_TYPE_FRAGMENT, fragmentPath, fragmentDefinitions, numDefinitions[ PLG_SHADER_TYPE_FRAGMENT ] );

	if ( !PlgLinkShaderProgram( program.internalPtr ) )
	{
		PRINT_WARNING( "Failed to link shader stages!\nPL: %s\n", PlGetError() );
		PlgDestroyShaderProgram( program.internalPtr, true );
		return NULL;
	}

	/* the default pass is an optional field that can outline
	 * the initial properties that should be used during a draw.
	 * a material can of course overwrite these. */
	child = NL_GetChildByName( root, "defaultPass" );
	if ( child != NULL )
	{
		/* need to assign this for variable validation */
		program.defaultPass.program = program.internalPtr;
		/* and now we can fill this out */
		YnCore_Material_ParsePass( child, &program.defaultPass );
	}

	/* allocate and return our program index */
	YNCoreShaderProgramIndex *out = PlMAlloc( sizeof( YNCoreShaderProgramIndex ), true );
	*out = program;
	return out;
}

static void RS_LoadShaderProgram( const char *path, void *userData )
{
	PRINT( "Loading program: \"%s\"\n", path );

	NLNode *root = NL_LoadFile( path, "program" );
	if ( root == NULL )
	{
		PRINT_WARNING( "Failed to load shader program \"%s\"!\nPL: %s\n", path, PlGetError() );
		return;
	}

	YNCoreShaderProgramIndex *program = RS_ParseShaderProgram( root );

	NL_DestroyNode( root );

	if ( program == NULL )
	{
		PRINT_WARNING( "An error occurred while loading shader program \"%s\"!\n", path );
		return;
	}

	strncpy( program->path, path, sizeof( program->path ) );
	program->node = PlInsertLinkedListNode( shaderPrograms, program );
}

YNCoreShaderProgramIndex *YnCore_GetShaderProgramByName( const char *name )
{
	PLLinkedListNode *root = PlGetFirstNode( shaderPrograms );
	while ( root != NULL )
	{
		YNCoreShaderProgramIndex *programIndex = PlGetLinkedListNodeUserData( root );
		if ( strcmp( name, programIndex->internalName ) == 0 )
			return programIndex;

		root = PlGetNextLinkedListNode( root );
	}

	return NULL;
}

void YR_Shader_Initialize( void )
{
	shaderPrograms = PlCreateLinkedList();
	if ( shaderPrograms == NULL )
		PRINT_ERROR( "Failed to create shader program list: %s\n", PlGetError() );

	PRINT( "Scanning for shader programs...\n" );

	PlScanDirectory( "materials/shaders", "node", RS_LoadShaderProgram, false, NULL );
	PlScanDirectory( "materials/shaders", "n", RS_LoadShaderProgram, false, NULL );

	PRINT( "%d shader programs indexed\n", PlGetNumLinkedListNodes( shaderPrograms ) );

	/* now fetch the default programs */
	const char *defaultShaderNames[ RS_MAX_DEFAULT_SHADERS ] = {
			[RS_SHADER_DEFAULT] = "default",
			[RS_SHADER_LIGHTING_PASS] = "base_lighting",
			[RS_SHADER_DEFAULT_VERTEX] = "default_vertex",
			[RS_SHADER_DEFAULT_ALPHA] = "default_alpha",
	};
	for ( unsigned int i = 0; i < RS_MAX_DEFAULT_SHADERS; ++i )
	{
		YNCoreShaderProgramIndex *programIndex = YnCore_GetShaderProgramByName( defaultShaderNames[ i ] );
		if ( programIndex == NULL )
			PRINT_ERROR( "Failed to find default shader program, \"%s\"!\n", defaultShaderNames[ i ] );

		defaultShaderPrograms[ i ] = programIndex->internalPtr;
	}
}
