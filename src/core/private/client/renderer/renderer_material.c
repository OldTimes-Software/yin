// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_linkedlist.h>

#include "core_private.h"
#include "renderer.h"
#include "renderer_material.h"
#include "world.h"
#include "game_interface.h"

#include <yin/node.h>
#include "gui_public.h"

static PLLinkedList *materials[ YN_CORE_MAX_CACHE_GROUPS ];

static PLGTexture *specularFallbackTexture;
static PLGTexture *normalFallbackTexture;
static PLGTexture *previewFallbackTexture;

typedef struct YNCoreMaterial
{
	char path[ PL_SYSTEM_MAX_PATH ];
	YNCoreMaterialPass passes[ MAX_MATERIAL_PASSES ];
	unsigned int numPasses;
	bool isCached;      // if false, it's just the preview
	PLGTexture *preview;// preview utilised for editor
	PLLinkedListNode *node;

	YNCoreMemoryReference mem;
} YNCoreMaterial;

static YNCoreMaterial *fallbackMaterial;

YNCoreMaterial *YnCore_GetFallbackMaterial( void )
{
	return fallbackMaterial;
}

void YnCore_InitializeMaterialSystem( void )
{
	PRINT( "Initializing material system\n" );

	for ( unsigned int i = 0; i < YN_CORE_MAX_CACHE_GROUPS; ++i )
	{
		materials[ i ] = PlCreateLinkedList();
		if ( materials[ i ] == NULL )
			PRINT_ERROR( "Failed to create materials list: %s\n", PlGetError() );
	}

	normalFallbackTexture   = YnCore_LoadTexture( "materials/shaders/textures/normal.tga", PLG_TEXTURE_FILTER_LINEAR );
	specularFallbackTexture = YnCore_LoadTexture( "materials/shaders/textures/black.png", PLG_TEXTURE_FILTER_LINEAR );
	previewFallbackTexture  = YnCore_LoadTexture( "materials/editor/no_preview.png", PLG_TEXTURE_FILTER_NEAREST );

	/* go ahead and create the fallback material */
	fallbackMaterial = PL_NEW( YNCoreMaterial );
	/* setup passes */
	fallbackMaterial->numPasses                  = 1;
	fallbackMaterial->preview                    = previewFallbackTexture;
	fallbackMaterial->isCached                   = true;
	fallbackMaterial->passes[ 0 ].program        = defaultShaderPrograms[ RS_SHADER_DEFAULT_VERTEX ];
	fallbackMaterial->passes[ 0 ].blendMode[ 0 ] = PLG_BLEND_NONE;
	fallbackMaterial->passes[ 0 ].blendMode[ 1 ] = PLG_BLEND_NONE;
	/* setup variables */
	fallbackMaterial->passes[ 0 ].numVariables                = 1;
	fallbackMaterial->passes[ 0 ].variables[ 0 ].type         = MATERIAL_VAR_TEXTURE;
	fallbackMaterial->passes[ 0 ].variables[ 0 ].data.userPtr = YnCore_GetFallbackTexture();
}

void YnCore_ShutdownMaterialSystem( void )
{
	/* Flush any objects pending deletion in case they are holding a material handle. */
	MemoryManager_FlushUnreferencedResources();

	unsigned int totalCachedMaterials = 0;
	unsigned int orphanedCaches       = 0;

	for ( unsigned int i = 0; i < YN_CORE_MAX_CACHE_GROUPS; ++i )
	{
		unsigned int cached_materials = PlGetNumLinkedListNodes( materials[ i ] );
		totalCachedMaterials += cached_materials;

		if ( cached_materials == 0 )
		{
			/* and now destroy the list */
			PlDestroyLinkedList( materials[ i ] );
			materials[ i ] = NULL;
		}
		else
			++orphanedCaches;
	}

	if ( totalCachedMaterials > 0 )
		PRINT_WARNING( "Shutting down material system with %u active materials, orphaned %u caches!\n",
		               totalCachedMaterials, orphanedCaches );
}

const char *YnCore_Material_GetPath( const YNCoreMaterial *material )
{
	return material->path;
}

PLGTexture *YnCore_Material_GetPreviewTexture( YNCoreMaterial *material )
{
	return material->preview;
}

/**
 * Convert the given tag into a blend mode type.
 */
static int RM_GetBlendModeByTag( const char *tag )
{
	static const char *blendModeTags[] = {
	        [PLG_BLEND_NONE]                = "none",
	        [PLG_BLEND_ZERO]                = "zero",
	        [PLG_BLEND_ONE]                 = "one",
	        [PLG_BLEND_SRC_COLOR]           = "src_color",
	        [PLG_BLEND_ONE_MINUS_SRC_COLOR] = "one_minus_src_color",
	        [PLG_BLEND_SRC_ALPHA]           = "src_alpha",
	        [PLG_BLEND_ONE_MINUS_SRC_ALPHA] = "one_minus_src_alpha",
	        [PLG_BLEND_DST_ALPHA]           = "dst_alpha",
	        [PLG_BLEND_ONE_MINUS_DST_ALPHA] = "one_minus_dst_alpha",
	        [PLG_BLEND_DST_COLOR]           = "dst_color",
	        [PLG_BLEND_ONE_MINUS_DST_COLOR] = "one_minus_dst_color",
	        [PLG_BLEND_SRC_ALPHA_SATURATE]  = "src_alpha_saturate",
	};
	PL_STATIC_ASSERT( PL_ARRAY_ELEMENTS( blendModeTags ) == PLG_MAX_BLEND_MODES, "" );

	for ( int i = 0; i < PLG_MAX_BLEND_MODES; ++i )
	{
		if ( strcmp( tag, blendModeTags[ i ] ) != 0 )
			continue;

		return i;
	}

	PRINT_WARNING( "Invalid blend mode specified, \"%s\", defaulting to \"none\"!\n", tag );
	return PLG_BLEND_NONE;
}

/**
 * Convert the given tag into it's built-in type.
 */
static MaterialBuiltinVar GetBuiltInByTag( const char *tag )
{
	static const char *builtInTags[] = {
	        [MATERIAL_BUILTIN_TIME]          = "time",
	        [MATERIAL_BUILTIN_DEPTH]         = "depth",
	        [MATERIAL_BUILTIN_VIEWPORT_SIZE] = "vpsize",
	};
	PL_STATIC_ASSERT( PL_ARRAY_ELEMENTS( builtInTags ) == MAX_MATERIAL_BUILTINS, "" );

	for ( int i = 0; i < MAX_MATERIAL_BUILTINS; ++i )
	{
		if ( strcmp( tag, builtInTags[ i ] ) != 0 )
			continue;

		return i;
	}

	return MATERIAL_BUILTIN_INVALID;
}

/**
 * Ensures that the material variable is set up in such a way it can
 * actually be applied for the uniform it's pointing to. Also known
 * as a shit block of code.
 */
static bool ValidateMaterialVariable( MaterialVariable *variable, PLGShaderUniformType uniformType )
{
	switch ( variable->type )
	{
		default:
			break;

		case MATERIAL_VAR_FLOAT: return ( uniformType == PLG_UNIFORM_FLOAT );
		case MATERIAL_VAR_DOUBLE: return ( uniformType == PLG_UNIFORM_DOUBLE );

		case MATERIAL_VAR_INT: return ( uniformType == PLG_UNIFORM_INT );
		case MATERIAL_VAR_UINT: return ( uniformType == PLG_UNIFORM_UINT );

		case MATERIAL_VAR_BOOL: return ( uniformType == PLG_UNIFORM_BOOL );

		case MATERIAL_VAR_VEC2: return ( uniformType == PLG_UNIFORM_VEC2 );
		case MATERIAL_VAR_VEC3: return ( uniformType == PLG_UNIFORM_VEC3 );
		case MATERIAL_VAR_VEC4: return ( uniformType == PLG_UNIFORM_VEC4 );

		case MATERIAL_VAR_MAT3: return ( uniformType == PLG_UNIFORM_MAT3 );
		case MATERIAL_VAR_MAT4:
			return ( uniformType == PLG_UNIFORM_MAT4 );

			/* special types */
		case MATERIAL_VAR_RENDERTARGET:
		case MATERIAL_VAR_TEXTURE:
		{
			return ( ( uniformType == PLG_UNIFORM_SAMPLER1D ) ||
			         ( uniformType == PLG_UNIFORM_SAMPLER1DSHADOW ) ||
			         ( uniformType == PLG_UNIFORM_SAMPLER2D ) ||
			         ( uniformType == PLG_UNIFORM_SAMPLER2DSHADOW ) ||
			         ( uniformType == PLG_UNIFORM_SAMPLERCUBE ) );
		}
		case MATERIAL_VAR_BUILTIN: return true;
	}

	return false;
}

/**
 * Iterate through each of the parameters provided in the 'shaderParameters'
 * block of the material.
 */
static void ParseShaderParameters( YNCoreMaterialPass *materialPass, YNNodeBranch *root )
{
	YNNodeBranch *node = YnNode_GetFirstChild( root );
	while ( node != NULL )
	{
		/* fetch the next node, so we can roll onto the next element early */
		YNNodeBranch *next = YnNode_GetNextChild( node );

		MaterialVariable *materialVariable = &materialPass->variables[ materialPass->numVariables ];

		/* validate that the property actually exists or is at least exposed by the shader.
		 * in the long-term we'll be doing this against our own shader program object, but
		 * for now, just do it directly against the shader itself */
		const char *propertyName      = YnNode_GetName( node );
		materialVariable->programSlot = PlgGetShaderUniformSlot( materialPass->program, propertyName );
		if ( materialVariable->programSlot == -1 )
		{
			PRINT_WARNING( "Failed to fetch uniform slot for variable \"%s\"!\n", propertyName );
			node = next;
			continue;
		}

		snprintf( materialVariable->name, sizeof( materialVariable->name ), "%s", propertyName );

		PLGShaderUniformType uniformType = PlgGetShaderUniformType( materialPass->program, materialVariable->programSlot );

		/* if it's a string, it *could* be a built-in type */
		if ( YnNode_GetType( node ) == YN_NODE_PROP_STR )
		{
			PLPath value;
			YnNode_GetStr( node, value, sizeof( value ) );
			if ( *value == '_' )
			{
				const char *p = ( value + 1 );
				// Render targets are "special" in the sense that we can specify what we want
				if ( strncmp( p, "rt_", 3 ) == 0 )
				{
					p += 3;
					YNCoreRenderTarget *renderTarget = YnCore_RenderTarget_GetByKey( p );
					if ( renderTarget == NULL )
					{// Passing flag of 0 to create a placeholder
						renderTarget = YnCore_RenderTarget_Create( p, 64, 64, 0 );
					}

					materialVariable->type         = MATERIAL_VAR_RENDERTARGET;
					materialVariable->data.userPtr = renderTarget;
				}
				else
				{
					/* lookup what it actually is */
					MaterialBuiltinVar materialBuiltinVar = GetBuiltInByTag( p );
					if ( materialBuiltinVar == MATERIAL_BUILTIN_INVALID )
					{
						PRINT_WARNING( "Invalid built-in variable, \"%s\", specified!\n", value );
						node = next;
						continue;
					}

					/* todo: consider validating the built-in type here, but for now, we won't bother... */
					materialVariable->type     = MATERIAL_VAR_BUILTIN;
					materialVariable->data.i32 = materialBuiltinVar;
				}
			}
		}

		/* otherwise, we need to handle it as a traditional var */
		if ( materialVariable->type == MATERIAL_VAR_INVALID )
		{
			/* now we need to convert from the node type to our internal
		 	 * material type, which is gross and sloppy and crap, but oh
		 	 * well! */
			switch ( uniformType )
			{
				default:
					break;

				case PLG_UNIFORM_BOOL:
				{
					if ( YnNode_GetBool( node, &materialVariable->data.boolean ) != YN_NODE_ERROR_SUCCESS )
						break;

					materialVariable->type = MATERIAL_VAR_BOOL;
					break;
				}

				case PLG_UNIFORM_FLOAT:
				{
					if ( YnNode_GetF32( node, &materialVariable->data.f32 ) != YN_NODE_ERROR_SUCCESS )
						break;

					materialVariable->type = MATERIAL_VAR_FLOAT;
					break;
				}
				case PLG_UNIFORM_DOUBLE:
				{
					if ( YnNode_GetF64( node, &materialVariable->data.f64 ) != YN_NODE_ERROR_SUCCESS )
						break;

					materialVariable->type = MATERIAL_VAR_DOUBLE;
					break;
				}

				case PLG_UNIFORM_UINT:
				{
					if ( YnNode_GetUI32( node, &materialVariable->data.ui32 ) != YN_NODE_ERROR_SUCCESS )
						break;

					materialVariable->type = MATERIAL_VAR_UINT;
					break;
				}
				case PLG_UNIFORM_INT:
				{
					if ( YnNode_GetI32( node, &materialVariable->data.i32 ) != YN_NODE_ERROR_SUCCESS )
						break;

					materialVariable->type = MATERIAL_VAR_INT;
					break;
				}

				case PLG_UNIFORM_SAMPLER1D:
				case PLG_UNIFORM_SAMPLER2D:
				case PLG_UNIFORM_SAMPLER3D:
				case PLG_UNIFORM_SAMPLERCUBE:
				case PLG_UNIFORM_SAMPLER1DSHADOW:
				case PLG_UNIFORM_SAMPLER2DSHADOW:
				{
					PLPath texturePath;
					if ( YnNode_GetStr( node, texturePath, sizeof( PLPath ) ) != YN_NODE_ERROR_SUCCESS )
						break;

					if ( pl_strcasecmp( materialVariable->name, "diffuseMap" ) == 0 )
						materialVariable->hint = RM_VAR_HINT_DIFFUSE;
					else if ( pl_strcasecmp( materialVariable->name, "normalMap" ) == 0 )
						materialVariable->hint = RM_VAR_HINT_NORMAL;
					else if ( pl_strcasecmp( materialVariable->name, "specularMap" ) == 0 )
						materialVariable->hint = RM_VAR_HINT_SPECULAR;

					materialVariable->type         = MATERIAL_VAR_TEXTURE;
					materialVariable->data.userPtr = YnCore_LoadTexture( texturePath, materialPass->textureFilter );
					break;
				}
			}

			if ( materialVariable->type == MATERIAL_VAR_INVALID )
			{
				PRINT_WARNING( "Invalid property type for shader variable \"%s\"!\n", propertyName );
				node = next;
				continue;
			}
		}

		if ( !ValidateMaterialVariable( materialVariable, uniformType ) )
		{
			PRINT_WARNING( "Mismatch between material variable type and uniform type!\n" );
			node = next;
			continue;
		}

		materialPass->numVariables++;

		node = next;
	}
}

void YnCore_Material_ParsePass( YNNodeBranch *root, YNCoreMaterialPass *materialPass )
{
	/* fetch the blend mode we'll use for the pass */
	YNNodeBranch *subNode;
	if ( ( subNode = YnNode_GetChildByName( root, "blendMode" ) ) != NULL )
	{
		const char *blendModesArray[ 2 ];
		if ( YnNode_GetStrArray( subNode, blendModesArray, 2 ) == YN_NODE_ERROR_SUCCESS )
		{
			materialPass->blendMode[ 0 ] = RM_GetBlendModeByTag( blendModesArray[ 0 ] );
			materialPass->blendMode[ 1 ] = RM_GetBlendModeByTag( blendModesArray[ 1 ] );
		}
		else
			PRINT_WARNING( "Invalid blend mode array in material!\n" );
	}

	materialPass->depthTest = YnNode_GetBoolByName( root, "depthTest", materialPass->depthTest );
	materialPass->cullMode  = YnNode_GetI32ByName( root, "cullMode", materialPass->cullMode );

	const char *textureFilterPtr = YnNode_GetStringByName( root, "textureFilterMode", NULL );
	if ( textureFilterPtr != NULL )
	{
		if ( pl_strcasecmp( textureFilterPtr, "mipmap_nearest" ) == 0 )
			materialPass->textureFilter = PLG_TEXTURE_FILTER_MIPMAP_NEAREST;
		else if ( pl_strcasecmp( textureFilterPtr, "mipmap_linear" ) == 0 )
			materialPass->textureFilter = PLG_TEXTURE_FILTER_MIPMAP_LINEAR;
		else if ( pl_strcasecmp( textureFilterPtr, "mipmap_linear_nearest" ) == 0 )
			materialPass->textureFilter = PLG_TEXTURE_FILTER_MIPMAP_LINEAR_NEAREST;
		else if ( pl_strcasecmp( textureFilterPtr, "mipmap_nearest_linear" ) == 0 )
			materialPass->textureFilter = PLG_TEXTURE_FILTER_MIPMAP_NEAREST_LINEAR;
		else if ( pl_strcasecmp( textureFilterPtr, "nearest" ) == 0 )
			materialPass->textureFilter = PLG_TEXTURE_FILTER_NEAREST;
		else if ( pl_strcasecmp( textureFilterPtr, "linear" ) == 0 )
			materialPass->textureFilter = PLG_TEXTURE_FILTER_LINEAR;
	}
	else
		materialPass->textureFilter = PLG_TEXTURE_FILTER_MIPMAP_LINEAR;

	/* now handle any specific parameters the material provides */
	if ( ( subNode = YnNode_GetChildByName( root, "shaderParameters" ) ) != NULL )
	{
		/* there's some extra complexity when parsing in parameters, so we'll defer that
		 * to another function */
		ParseShaderParameters( materialPass, subNode );
	}
	/* not sure whether the above section should be required yet, there might be
	 * a case where we only want to use the shader defaults? */
}

static YNCoreMaterial *ParseMaterial( YNCoreMaterial *material, YNNodeBranch *root, bool preview )
{
	// see if the preview texture is specified
	if ( material->preview == NULL )
	{
		material->preview          = previewFallbackTexture;
		const char *previewTexture = YnNode_GetStringByName( root, "previewTexture", NULL );
		if ( previewTexture != NULL )
			material->preview = YnCore_LoadTexture( previewTexture, PLG_TEXTURE_FILTER_MIPMAP_LINEAR );
	}

	// If it's just the preview we want, then stop here
	if ( preview )
		return material;

	/* each pass specifies how the object should be drawn before
	 * drawing it again and again for each child */
	YNNodeBranch *node;
	if ( ( node = YnNode_GetChildByName( root, "passes" ) ) != NULL )
	{
		node = YnNode_GetFirstChild( node );
		while ( node != NULL )
		{
			YNCoreMaterialPass *currentPass = &material->passes[ material->numPasses++ ];
			/* current pass should've already been cleared by prior memset,
			 * so no need to reset the state for some crap */

			/* fetch the shader program we need to use for this pass */
			const char *programName          = YnNode_GetStringByName( node, "shaderProgram", "default" );
			YNCoreShaderProgramIndex *programIndex = YnCore_GetShaderProgramByName( programName );
			if ( programIndex == NULL )
			{
				currentPass->program = defaultShaderPrograms[ RS_SHADER_DEFAULT ];
				PRINT_WARNING( "Failed to find program \"%s\", using fallback!\n", programName );
			}
			else
			{
				*currentPass         = programIndex->defaultPass;
				currentPass->program = programIndex->internalPtr;
			}

			YnCore_Material_ParsePass( node, currentPass );

			node = YnNode_GetNextChild( node );
		}
	}

	if ( material->numPasses == 0 )
		PRINT_WARNING( "No passes specified for material!\n" );

	material->isCached = true;

	return material;
}

static YNCoreMaterial *GetMaterial( const char *path, YNCoreCacheGroup group )
{
	PLLinkedListNode *node = PlGetFirstNode( materials[ group ] );
	while ( node != NULL )
	{
		YNCoreMaterial *material = PlGetLinkedListNodeUserData( node );
		if ( strcmp( material->path, path ) == 0 )
			return material;

		node = PlGetNextLinkedListNode( node );
	}

	return NULL;
}

static void DestroyMaterial( YNCoreMaterial *material )
{
	if ( material == NULL )
		return;

	for ( unsigned int i = 0; i < material->numPasses; ++i )
	{
		for ( unsigned int j = 0; j < material->passes[ i ].numVariables; ++j )
		{
			switch ( material->passes[ i ].variables[ j ].type )
			{
				case MATERIAL_VAR_TEXTURE:
					//TODO: right now this is all using the plgtexture crap directly, so... waaaahh!!!
					break;
				case MATERIAL_VAR_RENDERTARGET:
					YnCore_RenderTarget_Release( ( YNCoreRenderTarget * ) material->passes[ i ].variables[ j ].data.userPtr );
					break;
				default:
					break;
			}
		}
	}

	PLLinkedList *container = PlGetLinkedListNodeContainer( material->node );
	if ( container != NULL )
		PlDestroyLinkedListNode( material->node );

	PL_DELETE( material );
}

static void DestroyMaterialCallback( void *userData )
{
	DestroyMaterial( ( YNCoreMaterial * ) userData );
}

YNCoreMaterial *YnCore_Material_Cache( const char *path, YNCoreCacheGroup group, bool useFallback, bool preview )
{
	/* check if it's already cached */
	YNCoreMaterial *material = GetMaterial( path, group );
	if ( material != NULL )
	{
		// If it's not cached, and we're not asking for the preview, load the full thing
		if ( !material->isCached && !preview )
		{
			YNNodeBranch *root = YnNode_LoadFile( path, "material" );
			if ( root != NULL )
			{
				ParseMaterial( material, root, false );
				YnNode_DestroyBranch( root );
			}
			else
				PRINT_WARNING( "Failed to cache material, \"%s\" (%s)!\n", path, YnNode_GetErrorMessage() );
		}
		MemoryManager_AddReference( &material->mem );
		return material;
	}

	/* fallback should be optional, as in some cases we might actually care */
	YNCoreMaterial *fallbackPtr = useFallback ? fallbackMaterial : NULL;

	YNNodeBranch *root = YnNode_LoadFile( path, "material" );
	if ( root == NULL )
	{
		PRINT_WARNING( "Failed to load material, \"%s\" (%s)!\n", path, YnNode_GetErrorMessage() );
		return fallbackPtr;
	}

	material = PL_NEW( YNCoreMaterial );
	ParseMaterial( material, root, preview );

	YnNode_DestroyBranch( root );

	snprintf( material->path, sizeof( material->path ), "%s", path );
	material->node = PlInsertLinkedListNode( materials[ group ], material );

	MemoryManager_SetupReference( "material", MEM_CACHE_MATERIALS, &material->mem, DestroyMaterialCallback, material );
	MemoryManager_AddReference( &material->mem );

	return material;
}

void YnCore_Material_Release( YNCoreMaterial *material )
{
	assert( material != NULL );

	/* Fallback material isn't owned by the memory manager. */
	if ( material == fallbackMaterial )
		return;

	MemoryManager_ReleaseReference( &material->mem );
}

static void SetBuiltInVariable( PLGShaderProgram *program, int uniformSlot, int variable, unsigned int *curUnit )
{
	if ( variable == -1 )
		return;

	switch ( variable )
	{
		case MATERIAL_BUILTIN_TIME:
		{
			unsigned int numTicks = YnCore_GetNumTicks();
			PlgSetShaderUniformValueByIndex( program, uniformSlot, &numTicks, false );
			break;
		}

		case MATERIAL_BUILTIN_DEPTH:
		{
			PLGTexture *depthTexture = YnCore_GetPrimaryDepthAttachment();
			if ( depthTexture == NULL )
				break;

			PlgSetTexture( depthTexture, *curUnit );
			PlgSetShaderUniformValueByIndex( program, uniformSlot, curUnit, false );
			*curUnit++;
			break;
		}

		case MATERIAL_BUILTIN_VIEWPORT_SIZE:
		{
			int w, h;
			YnCore_Get2DViewportSize( &w, &h );
			PlgSetShaderUniformValueByIndex( program, uniformSlot, &PLVector2( ( float ) w, ( float ) h ), false );
			break;
		}

		default:
			break;
	}
}

static void SetGlobalUniforms( PLGShaderProgram *program, YNCoreLight *lights, unsigned int numLights )
{
	int slot;

	YNCoreWorld *world = Game_GetCurrentWorld();
	if ( world != NULL )
	{
		if ( ( slot = PlgGetShaderUniformSlot( program, "sun.colour" ) ) >= 0 )
			PlgSetShaderUniformValueByIndex( program, slot, &world->sunColour, false );
		if ( ( slot = PlgGetShaderUniformSlot( program, "sun.position" ) ) >= 0 )
			PlgSetShaderUniformValueByIndex( program, slot, &world->sunPosition, false );
		if ( ( slot = PlgGetShaderUniformSlot( program, "sun.ambience" ) ) >= 0 )
			PlgSetShaderUniformValueByIndex( program, slot, &world->ambience, false );

		if ( ( slot = PlgGetShaderUniformSlot( program, "fogColour" ) ) >= 0 )
			PlgSetShaderUniformValueByIndex( program, slot, &world->fogColour, false );
		if ( ( slot = PlgGetShaderUniformSlot( program, "fogNear" ) ) >= 0 )
			PlgSetShaderUniformValueByIndex( program, slot, &world->fogNear, false );
		if ( ( slot = PlgGetShaderUniformSlot( program, "fogFar" ) ) >= 0 )
			PlgSetShaderUniformValueByIndex( program, slot, &world->fogFar, false );
	}

	if ( ( slot = PlgGetShaderUniformSlot( program, "numLights" ) ) >= 0 )
	{
		PlgSetShaderUniformValueByIndex( program, slot, &numLights, false );
		for ( unsigned int i = 0; i < numLights; ++i )
		{
			/* todo: this would be a lot less fucking disturbing if we were using a proper static layout! */
			char buf[ 32 ] = "lights[0].";
			buf[ 7 ]       = '0' + i;
			char *p        = &buf[ 10 ];

			strcpy( p, "colour" );
			PlgSetShaderUniformValue( program, buf, &lights[ i ].colour, false );

			strcpy( p, "position" );
			PlgSetShaderUniformValue( program, buf, &lights[ i ].position, false );

			strcpy( p, "radius" );
			PlgSetShaderUniformValue( program, buf, &lights[ i ].radius, false );
		}
	}
}

void YnCore_Material_DrawMesh( YNCoreMaterial *material, PLGMesh *mesh, YNCoreLight *lights, unsigned int numLights )
{
	// If it's not had a full cache, use the fallback,
	// though ideally this shouldn't happen!
	assert( material->isCached );
	if ( !material->isCached )
	{
		fallbackMaterial->passes[ 0 ].variables[ 0 ].data.userPtr = material->preview;
		material                                                  = fallbackMaterial;
	}

	for ( unsigned int i = 0; i < material->numPasses; ++i )
	{
		YNCoreMaterialPass *curPass = &material->passes[ i ];

		PlgSetShaderProgram( curPass->program );
		PlgSetBlendMode( curPass->blendMode[ 0 ], curPass->blendMode[ 1 ] );

		// Mirror mode requires flipping the matrix,
		// so we'll need to update the cull mode
		PLGCullMode cullMode;
		if ( rendererState.mirror && ( rendererState.depth % 2 ) )
		{
			if ( curPass->cullMode == PLG_CULL_NEGATIVE )
				cullMode = PLG_CULL_POSITIVE;
			else if ( curPass->cullMode == PLG_CULL_POSITIVE )
				cullMode = PLG_CULL_NEGATIVE;
			else
				cullMode = curPass->cullMode;
		}
		else
			cullMode = curPass->cullMode;

		PlgSetCullMode( cullMode );

		PlgSetShaderUniformValue( curPass->program, "pl_model", PlGetMatrix( PL_MODELVIEW_MATRIX ), true );

		SetGlobalUniforms( curPass->program, lights, numLights );

		unsigned int curUnit = 0;
		for ( unsigned int j = 0; j < curPass->numVariables; ++j )
		{
			if ( curPass->variables[ j ].type == MATERIAL_VAR_BUILTIN )
			{
				SetBuiltInVariable( curPass->program, curPass->variables[ j ].programSlot, curPass->variables[ j ].data.i32, &curUnit );
				continue;
			}
			// textures just need to be set per their respective unit
			else if ( curPass->variables[ j ].type == MATERIAL_VAR_TEXTURE || curPass->variables[ j ].type == MATERIAL_VAR_RENDERTARGET )
			{
				PL_GET_CVAR( "r.skipDiffuse", skipDiffuse );
				if ( skipDiffuse != NULL && ( curPass->variables[ j ].hint == RM_VAR_HINT_DIFFUSE && skipDiffuse->b_value ) )
					continue;

				PLGTexture *texture;
				if ( curPass->variables[ j ].type == MATERIAL_VAR_RENDERTARGET )
				{
					texture = YnCore_RenderTarget_GetTextureAttachment( ( YNCoreRenderTarget * ) curPass->variables[ j ].data.userPtr );
					if ( texture == NULL )
						texture = YnCore_GetFallbackTexture();
				}
				else
					texture = ( PLGTexture * ) curPass->variables[ j ].data.userPtr;

				assert( texture != NULL );

				PL_GET_CVAR( "r.skipNormal", skipNormal );
				if ( skipNormal != NULL && ( curPass->variables[ j ].hint == RM_VAR_HINT_NORMAL && skipNormal->b_value ) )
					texture = normalFallbackTexture;

				PL_GET_CVAR( "r.skipSpecular", skipSpecular );
				if ( skipSpecular != NULL && ( curPass->variables[ j ].hint == RM_VAR_HINT_SPECULAR && skipSpecular->b_value ) )
					texture = specularFallbackTexture;

				PlgSetTexture( texture, curUnit );
				PlgSetTextureFilter( texture, curPass->textureFilter );

				PlgSetShaderUniformValueByIndex( curPass->program, curPass->variables[ j ].programSlot, &curUnit, false );
				curUnit++;
				continue;
			}

			PlgSetShaderUniformValueByIndex( curPass->program, curPass->variables[ j ].programSlot, &curPass->variables[ j ].data, false );
		}

		PlgUploadMesh( mesh );
		PlgDrawMesh( mesh );

		g_gfxPerfStats.numBatches++;
		if ( mesh->primitive == PLG_MESH_TRIANGLES )
			g_gfxPerfStats.numTriangles += mesh->num_triangles;
		else
			g_gfxPerfStats.numTriangles += ( mesh->num_verts / 2 );
	}

	PlgSetCullMode( PLG_CULL_POSITIVE );

	if ( !material->isCached )
		fallbackMaterial->passes[ 0 ].variables[ 0 ].data.userPtr = YnCore_GetFallbackTexture();
}
