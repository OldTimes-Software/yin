/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plgraphics/plg_driver_interface.h>

#include "core_private.h"
#include "legacy/actor.h"
#include "renderer_font.h"
#include "world.h"
#include "game_interface.h"
#include "renderer.h"
#include "renderer_particle.h"

#include "client/client_gui.h"
#include "editor/editor.h"

#include "post/post.h"

YNCoreRendererStats g_gfxPerfStats;
YNCoreRendererPassState rendererState;

static PLGCamera *auxCamera = NULL;

static PLGFrameBuffer *fboBuffer;

/* Post Processing */

void YnCore_SetupRenderTarget( PLGFrameBuffer **buffer, PLGTexture **attachment, PLGTexture **depthAttachment, unsigned int w, unsigned int h )
{
	unsigned int bw = 0, bh = 0;
	if ( *buffer != NULL )
		PlgGetFrameBufferResolution( *buffer, &bw, &bh );

	/* need to rebuild the framebuffer object
	 * todo: the library should provide us a func to perform a resize? */
	if ( bw != w || bh != h )
	{
		PlgDestroyFrameBuffer( *buffer );
		*buffer = PlgCreateFrameBuffer( w, h, PLG_BUFFER_COLOUR | PLG_BUFFER_DEPTH | PLG_BUFFER_STENCIL );
		if ( *buffer == NULL )
			PRINT_ERROR( "Failed to create framebuffer: %s\n", PlGetError() );

		PlgDestroyTexture( *attachment );
		*attachment = PlgGetFrameBufferTextureAttachment( *buffer, PLG_BUFFER_COLOUR, PLG_TEXTURE_FILTER_LINEAR );
		if ( *attachment == NULL )
			PRINT_ERROR( "Failed to create texture attachment: %s\n", PlGetError() );

		if ( depthAttachment != NULL )
		{
			PlgDestroyTexture( *depthAttachment );
			*depthAttachment = PlgGetFrameBufferTextureAttachment( *buffer, PLG_BUFFER_DEPTH | PLG_BUFFER_STENCIL, PLG_TEXTURE_FILTER_LINEAR );
			if ( *depthAttachment == NULL )
				PRINT_ERROR( "Failed to create depth attachment: %s\n", PlGetError() );
		}
	}

	/* reset */
	PlgBindFrameBuffer( NULL, PLG_FRAMEBUFFER_DRAW );
}

/**********************************************************/

static PLGTexture *numTextureTable[ 10 ];

void R_DrawDigit( float x, float y, int digit )
{
	if ( digit < 0 )
		digit = 0;
	else if ( digit > 9 )
		digit = 9;

	PlgDrawTexturedRectangle( x, y, ( float ) numTextureTable[ digit ]->w, ( float ) numTextureTable[ digit ]->h, numTextureTable[ digit ] );
}

void R_DrawNumber( float x, float y, int number )
{
	/* restrict it for sanity */
	if ( number < 0 )
		number = 0;
	else if ( number > 999 )
		number = 999;

	if ( number >= 100 )
	{
		int digit = number / 100;
		R_DrawDigit( x, y, digit );
		x += ( float ) numTextureTable[ digit ]->w + 1;
	}

	if ( number >= 10 )
	{
		int digit = ( number / 10 ) % 10;
		R_DrawDigit( x, y, digit );
		x += ( float ) numTextureTable[ digit ]->w + 1;
	}

	R_DrawDigit( x, y, number % 10 );
}

void YnCore_SetupDefaultRenderState( const YNCoreViewport *viewport )
{
	PLColour clearColour = { 50, 50, 50, 255 };

	YNCoreWorld *world = Game_GetCurrentWorld();
	if ( world != NULL && ( viewport->camera == NULL || viewport->camera->mode == YN_CORE_CAMERA_MODE_PERSPECTIVE ) )
	{
		clearColour = PlColourF32ToU8( &world->clearColour );
	}

	PlgSetClearColour( clearColour );

	PlgEnableGraphicsState( PLG_GFX_STATE_SCISSORTEST );

	PlgSetDepthBufferMode( PLG_DEPTHBUFFER_ENABLE );
	PlgSetDepthMask( true );

	PlgSetCullMode( PLG_CULL_POSITIVE );

	PlgSetShaderProgram( defaultShaderPrograms[ RS_SHADER_DEFAULT ] );
}

void YnCore_BeginDraw( YNCoreViewport *viewport )
{
	double newTime = PlGetCurrentSeconds();

	viewport->perf.frameReadings[ viewport->perf.frameIndex++ ] = 1.0 / ( newTime - viewport->perf.oldTime );
	if ( viewport->perf.frameIndex >= YN_CORE_MAX_FPS_READINGS )
	{
		viewport->perf.frameIndex = 0;
	}
	viewport->perf.oldTime = newTime;

	YnCore_SetupDefaultRenderState( viewport );

	PlgSetViewport( viewport->x, viewport->y, viewport->width, viewport->height );
	PlgClearBuffers( PLG_BUFFER_DEPTH | PLG_BUFFER_COLOUR );
}

void YnCore_EndDraw( YNCoreViewport *viewport )
{
	viewport->perf.numBatches   = 0;
	viewport->perf.numTriangles = 0;
	viewport->perf.numPolygons  = 0;
	viewport->perf.numPortals   = 0;
}

static YNCoreMaterial *ppFXAAMaterial = NULL;

void YR_Shader_Initialize( void );  /* renderer/shaders.c */
void RT_InitializeTextures( void ); /* texture.c */

/* renderer_rendertarget.c */
void YnCore_InitializeRenderTargets( void );
void YnCore_ShutdownRenderTargets( void );

void Renderer_RegisterConsoleVariables( void )
{
	PlRegisterConsoleVariable( "r.cullMode", "Face culling mode.", "1", PL_VAR_I32, NULL, NULL, false );
	PlRegisterConsoleVariable( "r.superSampling", "Resolution multiplier.", "1", PL_VAR_I32, NULL, NULL, true );
	PlRegisterConsoleVariable( "r.showActorBounds", "Toggle actor bounds.", "0", PL_VAR_BOOL, NULL, NULL, false );
	PlRegisterConsoleVariable( "r.showFPS", "Toggle FPS counter.", "false", PL_VAR_BOOL, NULL, NULL, true );
	PlRegisterConsoleVariable( "r.wireframe", "Enable wireframe mode.", "0", PL_VAR_BOOL, NULL, NULL, false );
	PlRegisterConsoleVariable( "r.skyHeightOffset", "Height of the sky relative to the camera.", "10", PL_VAR_F32, NULL, NULL, false );
	PlRegisterConsoleVariable( "r.skyCull", "Cull backfaces for the sky. Only useful if you set the offset lower than the camera.", "1", PL_VAR_BOOL, NULL, NULL, true );
	PlRegisterConsoleVariable( "r.skipDiffuse", "Skip diffuse map.", "0", PL_VAR_BOOL, NULL, NULL, false );
	PlRegisterConsoleVariable( "r.skipNormal", "Skip normal map.", "0", PL_VAR_BOOL, NULL, NULL, false );
	PlRegisterConsoleVariable( "r.skipSpecular", "Skip specular map.", "0", PL_VAR_BOOL, NULL, NULL, false );
	PlRegisterConsoleVariable( "r.driver", "Sets the default graphics driver. Requires restart.", "opengl", PL_VAR_STRING, NULL, NULL, true );

	// Camera
	PlRegisterConsoleVariable( "r.fov", "", "75", PL_VAR_F32, NULL, NULL, true );
	PlRegisterConsoleVariable( "r.near", "", "0.1", PL_VAR_F32, NULL, NULL, true );
	PlRegisterConsoleVariable( "r.far", "", "1000.0", PL_VAR_F32, NULL, NULL, true );
}

void YnCore_InitializeRenderer( void )
{
	PRINT( "Initializing renderer\n" );

	PL_ZERO_( rendererState );

	RT_InitializeTextures();

	YR_Shader_Initialize();
	YnCore_InitializeRenderTargets();
	YnCore_InitializeMaterialSystem();
	YR_Font_Initialize();

	auxCamera = PlgCreateCamera();
	if ( auxCamera == NULL )
		PRINT_ERROR( "Failed to create auxiliary camera: %s\n", PlGetError() );

	auxCamera->mode = PLG_CAMERA_MODE_ORTHOGRAPHIC;
	auxCamera->near = -10000.0f;
	auxCamera->far  = 10000.0f;

	//SetupShadowMap();
	YnCore_SetupDefaultRenderState( NULL );

	R_PP_SetupEffects();
}

void YnCore_ShutdownRenderer( void )
{
	Font_Shutdown();
	YnCore_ShutdownMaterialSystem();
	YnCore_ShutdownRenderTargets();
}

/**
 * Where the magic of post processing happens.
 */
static void DrawScenePost( const YNCoreViewport *viewport )
{
	PL_GET_CVAR( "r.postProcessing", postProcessingVar );
	if ( postProcessingVar == NULL || !postProcessingVar->b_value )
		return;

	R_PP_Draw( viewport );
}

void YR_DrawGraph( const char *heading, float x, float y, float w, float h, const double *values, unsigned int numPoints, float min, float max )
{
	if ( numPoints < 2 )
		return;

	PlgSetShaderProgram( defaultShaderPrograms[ RS_SHADER_DEFAULT_VERTEX ] );

	double oa = min, ob = max;
	for ( unsigned int i = 0; i < numPoints; ++i )
	{
		if ( values[ i ] > max )
			max = values[ i ];
		if ( values[ i ] < min )
			min = values[ i ];
	}

	bool outOfBounds = false;
	if ( oa != min || max != ob )
		outOfBounds = true;

	unsigned int numOutPoints = ( numPoints - 1 ) * 2;
	PLVector3   *points       = PlCAllocA( numOutPoints, sizeof( PLVector3 ) );

	/* convert the values we've been provided into points in our graph */
	for ( unsigned int i = 0, j = 1; j < numPoints; i++, j++ )
	{
		points[ i ].x = x + ( ( w / ( numPoints - 1 ) ) * ( j - 1 ) );
		if ( min != max )
			points[ i ].y = y + h - 1 - ( ( values[ j - 1 ] - min ) * ( h / ( max - min ) ) );

		++i;

		points[ i ].x = x + ( ( w / ( numPoints - 1 ) ) * j );
		if ( min != max )
			points[ i ].y = y + h - 1 - ( ( values[ j ] - min ) * ( h / ( max - min ) ) );

		/* leave z, it'll be initialized as 0 */
	}

	PlgSetBlendMode( PLG_BLEND_DEFAULT );
	PlgDrawRectangle( x, y, w, h, PLColour( 0, 0, 0, 200 ) );
	PlgSetBlendMode( PLG_BLEND_DISABLE );

#if 0
	PLVector3 border[] = {
	        { x, y, 0 },
	        { x + w, y, 0 },// top
	        { x, y, 0 },
	        { x, y + h, 0 },// left
	        { x + w, y + h, 0 },
	        { x, y + h, 0 },// bottom
	        { x + w, y + h, 0 },
	        { x + w, y, 0 },// right
	};
	PlgDrawLines( border, PL_ARRAY_ELEMENTS( border ), PL_COLOUR_GOLD );
#endif

	PlgDrawLines( points, numOutPoints, PL_COLOUR_WHITE );

	BitmapFont *font = Font_GetDefaultSmall();
	Font_BeginDraw( font );

	if ( heading != NULL )
	{
		size_t len  = strlen( heading );
		float  cPos = ( x + w - ( len * font->cw ) ) - 2.0f;
		Font_AddBitmapStringToPass( font, cPos, y + 2.0f, 1.0f, PLColourRGB( 0, 255, 0 ), heading, len, false );
	}

	// Calculate the average sum of all the points
	double avg = 0.0;
	for ( unsigned int i = 0; i < numPoints; ++i )
		avg += values[ i ];
	avg /= numPoints;

	char buf[ 128 ];

	// Current and average readings
	snprintf( buf, sizeof( buf ), "CUR %02f", values[ numPoints - 1 ] );
	Font_AddBitmapStringToPass( font, x + 2.0f, y + ( h / 2 ) - ( font->ch / 2 ) + font->ch, 1.0f, outOfBounds ? PL_COLOUR_INDIAN_RED : PL_COLOUR_SEA_GREEN, buf, strlen( buf ), false );
	snprintf( buf, sizeof( buf ), "AVG %02f", avg );
	Font_AddBitmapStringToPass( font, x + 2.0f, y + ( h / 2 ) - ( font->ch / 2 ) - font->ch, 1.0f, outOfBounds ? PL_COLOUR_INDIAN_RED : PL_COLOUR_SEA_GREEN, buf, strlen( buf ), false );

	snprintf( buf, sizeof( buf ), "y+:%02f", max );
	Font_AddBitmapStringToPass( font, x + 2.0f, y + 2.0f, 1.0f, outOfBounds ? PL_COLOUR_INDIAN_RED : PL_COLOUR_SEA_GREEN, buf, strlen( buf ), false );
	snprintf( buf, sizeof( buf ), "y-:%02f", min );
	Font_AddBitmapStringToPass( font, x + 2.0f, y + ( h - font->ch ) - 2.0f, 1.0f, outOfBounds ? PL_COLOUR_INDIAN_RED : PL_COLOUR_SEA_GREEN, buf, strlen( buf ), false );

	Font_Draw( font );

	PlFree( points );
}

static void DrawEditorOverlay( const YNCoreViewport *viewport )
{
	YNCoreEditorInstance *editorInstance = Editor_GetCurrentInstance();
	if ( editorInstance == NULL )
		return;

	const YNCoreCamera *camera = viewport->camera;
	if ( camera != NULL && ( camera->mode != YN_CORE_CAMERA_MODE_PERSPECTIVE ) && ( editorInstance->gridScale > 0 ) )
	{
		PlgSetShaderProgram( defaultShaderPrograms[ RS_SHADER_DEFAULT_VERTEX ] );

		static float z    = 16.0f;
		float        zoom = roundf( z ) / 2.0f;

		float x = 500.0f + sinf( zoom * 2.0f ) * 100.0f;
		float y = 200.0f + cosf( zoom * 2.0f ) * 100.0f;

		PLMatrix4 transform = PlMatrix4Identity();
		transform           = PlScaleMatrix4( transform, ( PLVector3 ){ zoom, zoom, zoom } );

		// stupid matrix bollocks, blargh
		transform = PlTransposeMatrix4( &transform );
		PlgSetViewMatrix( &transform );

		//todo: hook these up with vars...
		int m = ( viewport->width > viewport->height ) ? viewport->width : viewport->height;
		PlgDrawDottedGrid( -m / 2, -m / 2, m, m, editorInstance->gridScale / 2, &( PLColour ){ 70, 70, 70, 255 } );
		PlgDrawDottedGrid( -m / 2, -m / 2, m, m, ( editorInstance->gridScale / 2 ) * 4, &( PLColour ){ 100, 100, 100, 255 } );

		switch ( camera->mode )
		{
			default:
				break;
			case YN_CORE_CAMERA_MODE_TOP:
				transform = PlMultiplyMatrix4( transform, PlTranslateMatrix4( ( PLVector3 ){ x, -0.0f, -y } ) );
				transform = PlMultiplyMatrix4( transform, PlRotateMatrix4( PL_DEG2RAD( 90.0f ), &( PLVector3 ){ 1.0f, 0.0f, 0.0f } ) );
				break;
			case YN_CORE_CAMERA_MODE_LEFT:
				transform = PlMultiplyMatrix4( transform, PlTranslateMatrix4( ( PLVector3 ){ 0.0f, -y, -x } ) );
				transform = PlMultiplyMatrix4( transform, PlRotateMatrix4( PL_DEG2RAD( 90.0f ), &( PLVector3 ){ 0.0f, 1.0f, 0.0f } ) );
				transform = PlMultiplyMatrix4( transform, PlRotateMatrix4( PL_DEG2RAD( 180.0f ), &( PLVector3 ){ 0.0f, 0.0f, 1.0f } ) );
				break;
			case YN_CORE_CAMERA_MODE_FRONT:
				transform = PlMultiplyMatrix4( transform, PlTranslateMatrix4( ( PLVector3 ){ -x, -y, 0.0f } ) );
				transform = PlMultiplyMatrix4( transform, PlRotateMatrix4( PL_DEG2RAD( 180.0f ), &( PLVector3 ){ 0.0f, 0.0f, 1.0f } ) );
				break;
		}

		// stupid matrix bollocks, blargh
		transform = PlTransposeMatrix4( &transform );
		PlgSetViewMatrix( &transform );

		YNCoreCamera tmp;
		PL_ZERO_( tmp );
		tmp.internal = auxCamera;
		YnCore_World_DrawWireframe( Game_GetCurrentWorld(), &tmp );

		// reset the view matrix back to it's original state
		PlgSetViewMatrix( &auxCamera->internal.view );
	}

	BitmapFont *defaultFont = Font_GetDefault();
	if ( defaultFont == NULL )
		return;

	Font_BeginDraw( defaultFont );

	const char *label;
	if ( camera != NULL )
	{
		switch ( camera->mode )
		{
			default:
			case YN_CORE_CAMERA_MODE_FRONT:
				label = "Front";
				break;
			case YN_CORE_CAMERA_MODE_LEFT:
				label = "Left";
				break;
			case YN_CORE_CAMERA_MODE_PERSPECTIVE:
				label = "Perspective";
				break;
			case YN_CORE_CAMERA_MODE_TOP:
				label = "Top";
				break;
		}
	}
	else
		label = "No camera!";

	Font_AddBitmapStringToPass( defaultFont,
	                            ( float ) ( ( viewport->width - ( defaultFont->cw * 2 ) ) - ( defaultFont->cw * strlen( label ) ) ),
	                            ( float ) ( viewport->height - ( defaultFont->ch * 2 ) ),
	                            1.0f, PL_COLOUR_GOLD, label, strlen( label ), true );

	Font_Draw( defaultFont );
}

static void DrawDebugOverlay( const YNCoreViewport *viewport )
{
	PL_GET_CVAR( "debug.overlay", debugOverlay );
	if ( debugOverlay->i_value <= 0 )
		return;

	BitmapFont *defaultFont = Font_GetDefaultSmall();
	assert( defaultFont != NULL );
	if ( defaultFont == NULL )
		return;

	Font_BeginDraw( defaultFont );

	static const float sy = 8;
	static const float sx = 8;
	static const float tx = 8 + 4;
	float              y  = sy;

	const YNCoreCamera *camera = viewport->camera;
	if ( camera != NULL )
	{
		// Draw camera position
		char buf[ 128 ];
		PL_ZERO( buf, sizeof( buf ) );
		const char *vpos = PlPrintVector3( &camera->internal->position, PL_VAR_I32 );
		strcat( buf, vpos );
		strcat( buf, " (" );
		const char *vang = PlPrintVector3( &camera->internal->angles, PL_VAR_I32 );
		strcat( buf, vang );
		strcat( buf, ")" );
		Font_AddBitmapStringToPass( defaultFont, tx, y += defaultFont->ch, 1.0f, PL_COLOUR_WHITE, buf, strlen( buf ), false );
	}

	// Draw stats
	char buf[ 64 ];
	snprintf( buf, sizeof( buf ), "FPS:           " PL_FMT_uint32 "\n", YnCore_Viewport_GetAverageFPS( viewport ) );
	Font_AddBitmapStringToPass( defaultFont, tx, y += defaultFont->ch, 1.0f, PL_COLOUR_GOLD, buf, strlen( buf ), false );
	snprintf( buf, sizeof( buf ), "Num faces:     " PL_FMT_uint32 "\n", g_gfxPerfStats.numFacesDrawn );
	Font_AddBitmapStringToPass( defaultFont, tx, y += defaultFont->ch, 1.0f, PL_COLOUR_GOLD, buf, strlen( buf ), false );
	snprintf( buf, sizeof( buf ), "Num portals:   " PL_FMT_uint32 "\n", g_gfxPerfStats.numVisiblePortals );
	Font_AddBitmapStringToPass( defaultFont, tx, y += defaultFont->ch, 1.0f, PL_COLOUR_GOLD, buf, strlen( buf ), false );
	snprintf( buf, sizeof( buf ), "Num triangles: " PL_FMT_uint32 "\n", g_gfxPerfStats.numTriangles );
	Font_AddBitmapStringToPass( defaultFont, tx, y += defaultFont->ch, 1.0f, PL_COLOUR_GOLD, buf, strlen( buf ), false );
	snprintf( buf, sizeof( buf ), "Num batches:   " PL_FMT_uint32 "\n", g_gfxPerfStats.numBatches );
	Font_AddBitmapStringToPass( defaultFont, tx, y += defaultFont->ch, 1.0f, PL_COLOUR_GOLD, buf, strlen( buf ), false );
	snprintf( buf, sizeof( buf ), "Alloc memory:  %.2lfMB\n", PlBytesToMegabytes( PlGetTotalAllocatedMemory() ) );
	Font_AddBitmapStringToPass( defaultFont, tx, y += defaultFont->ch, 1.0f, PL_COLOUR_ORCHID, buf, strlen( buf ), false );
	snprintf( buf, sizeof( buf ), "Total memory:  %.2lfMB\n", PlBytesToMegabytes( PlGetCurrentMemoryUsage() ) );
	Font_AddBitmapStringToPass( defaultFont, tx, y += defaultFont->ch, 1.0f, PL_COLOUR_ORCHID, buf, strlen( buf ), false );

	unsigned int numTasks = Sch_GetNumTasks();
	snprintf( buf, sizeof( buf ), "Num tasks:     " PL_FMT_uint32 "\n", numTasks );
	Font_AddBitmapStringToPass( defaultFont, tx, y += defaultFont->ch, 1.0f, PL_COLOUR_MAGENTA, buf, strlen( buf ), false );
	for ( unsigned int i = 0; i < numTasks; ++i )
	{
		double      taskDelay;
		const char *taskDescription = Sch_GetTaskDescription( i, &taskDelay );
		snprintf( buf, sizeof( buf ), "%u %s\n", i, taskDescription );
		Font_AddBitmapStringToPass( defaultFont, tx + 8, y += defaultFont->ch, 1.0f, PL_COLOUR_MAGENTA, buf, strlen( buf ), false );
	}
	y += defaultFont->ch * 2;

	static const float bw = 128;

	PlgSetShaderProgram( defaultShaderPrograms[ RS_SHADER_DEFAULT_VERTEX ] );
	PlgSetBlendMode( PLG_BLEND_DEFAULT );
	PlgDrawRectangle( sx, sy, bw, y - sy, PLColour( 0, 0, 0, 200 ) );
	PlgSetBlendMode( PLG_BLEND_DISABLE );

	Font_Draw( defaultFont );

	if ( debugOverlay->i_value > 1 )
	{
		float x = sx;

		static const float graphHeight = 64;
		for ( unsigned int i = 0; i < MAX_PROFILER_GROUPS; ++i )
		{
			if ( y + graphHeight >= ( float ) viewport->height )
			{
				y = sy;
				x += bw;
			}

			uint8_t       numPoints;
			const double *graph = Profiler_GetGraph( i, &numPoints );
			YR_DrawGraph( cpuProfilerDescriptions[ i ], x, y, bw, graphHeight, graph, numPoints, .0f, 1.0f );
			y += graphHeight;
		}
	}
}

void YnCore_Set2DViewportSize( int w, int h )
{
	PlgSetViewport( 0, 0, w, h );
	PlgSetupCamera( auxCamera );
}

void YnCore_Get2DViewportSize( int *width, int *height )
{
	*width  = 640;
	*height = 480;
}

void YnCore_DrawMenu( const YNCoreViewport *viewport )
{
	YN_CORE_PROFILE_START( PROFILE_DRAW_UI );

	YnCore_Set2DViewportSize( viewport->width, viewport->height );

	PlgSetDepthMask( false );

	PlMatrixMode( PL_MODELVIEW_MATRIX );
	PlPushMatrix();
	PlLoadIdentityMatrix();

	DrawScenePost( NULL );

	//YRCamera camera;
	//PL_ZERO_( camera );
	//camera.internal = auxCamera;
	//YR_World_DrawWireframe( Game_GetCurrentWorld(), &camera );

	YnCore_DrawGUI( viewport );

	if ( viewport != NULL )
		DrawEditorOverlay( viewport );

	DrawDebugOverlay( viewport );

	PlgSetTexture( NULL, 0 );

	PlPopMatrix();

	PlgSetDepthMask( true );

	YN_CORE_PROFILE_END( PROFILE_DRAW_UI );

	PL_ZERO_( g_gfxPerfStats );
}

void YnCore_Draw2DQuad( YNCoreMaterial *material, int x, int y, int w, int h )
{
	static PLGMesh *mesh = NULL;
	if ( mesh == NULL )
		mesh = PlgCreateMesh( PLG_MESH_TRIANGLE_STRIP, PLG_DRAW_DYNAMIC, 2, 4 );

	PlgClearMesh( mesh );

	PlgAddMeshVertex( mesh, PlVector3( x, y + h, 0 ), pl_vecOrigin3, PL_COLOUR_WHITE, PlVector2( 0, 0 ) );
	PlgAddMeshVertex( mesh, PlVector3( x, y, 0 ), pl_vecOrigin3, PL_COLOUR_WHITE, PlVector2( 0, 1 ) );
	PlgAddMeshVertex( mesh, PlVector3( x + w, y + h, 0 ), pl_vecOrigin3, PL_COLOUR_WHITE, PlVector2( 1, 0 ) );
	PlgAddMeshVertex( mesh, PlVector3( x + w, y, 0 ), pl_vecOrigin3, PL_COLOUR_WHITE, PlVector2( 1, 1 ) );

	YnCore_Material_DrawMesh( material, mesh, NULL, 0 );
}

void YnCore_DrawAxesPivot( PLVector3 position, PLVector3 rotation )
{
	PlMatrixMode( PL_MODELVIEW_MATRIX );
	PlPushMatrix();

	PlLoadIdentityMatrix();

	PlgSetShaderProgram( defaultShaderPrograms[ RS_SHADER_DEFAULT_VERTEX ] );

	PLVector3 angles;
	angles.x = PL_DEG2RAD( rotation.x );
	angles.y = PL_DEG2RAD( rotation.y );
	angles.z = PL_DEG2RAD( rotation.z );

	PlRotateMatrix( angles.x, 1.0f, 0.0f, 0.0f );
	PlRotateMatrix( angles.y, 0.0f, 1.0f, 0.0f );
	PlRotateMatrix( angles.z, 0.0f, 0.0f, 1.0f );

	PlTranslateMatrix( position );

	PLMatrix4 transform = *PlGetMatrix( PL_MODELVIEW_MATRIX );
	PlgDrawSimpleLine( transform, PLVector3( 0, 0, 0 ), PLVector3( 10, 0, 0 ), PLColour( 255, 0, 0, 255 ) );
	PlgDrawSimpleLine( transform, PLVector3( 0, 0, 0 ), PLVector3( 0, 10, 0 ), PLColour( 0, 255, 0, 255 ) );
	PlgDrawSimpleLine( transform, PLVector3( 0, 0, 0 ), PLVector3( 0, 0, 10 ), PLColour( 0, 0, 255, 255 ) );
	//printf( "%s\n", PlPrintVector3( &position, pl_int_var ) );

	PlPopMatrix();
}

static void YR_RenderScene( YNCoreCamera *camera, const YNCoreViewport *viewport )
{
	YNCoreWorldSector *currentSector = NULL;
	YNCoreWorld *world         = Game_GetCurrentWorld();
	if ( world != NULL )
		currentSector = YnCore_World_GetSectorByGlobalOrigin( world, &camera->internal->position );

	/* todo:	
		this needs to be restructured, as drawing is going to
		depend on whatever sector is currently being drawn.
		but for v2, this'll suffice...
	*/

	if ( viewport != NULL )
	{
		if ( camera->mode == YN_CORE_CAMERA_MODE_PERSPECTIVE )
		{
			if ( camera->drawMode == YN_CORE_CAMERA_DRAW_MODE_WIREFRAME )
				YnCore_World_DrawWireframe( world, camera );
			else
				YnCore_World_Draw( world, currentSector, camera );

			YNCoreEditorInstance *editorInstance = Editor_GetCurrentInstance();
			if ( editorInstance != NULL && editorInstance->gridScale > 0 )
			{
				PlMatrixMode( PL_MODELVIEW_MATRIX );
				PlPushMatrix();

				PLVector3 angles;
				angles.x = PL_DEG2RAD( 90.0f );
				angles.y = PL_DEG2RAD( 0.0f );
				angles.z = PL_DEG2RAD( 0.0f );

				PlRotateMatrix( angles.x, 1.0f, 0.0f, 0.0f );
				PlRotateMatrix( angles.y, 0.0f, 1.0f, 0.0f );
				PlRotateMatrix( angles.z, 0.0f, 0.0f, 1.0f );

				PlTranslateMatrix( PLVector3( 0, -5, 0 ) );

				static const unsigned int gridW = 256;

				PlgSetShaderProgram( defaultShaderPrograms[ RS_SHADER_DEFAULT_VERTEX ] );
				PlgDrawDottedGrid( -( gridW / 2 ), -( gridW / 2 ), gridW, gridW, editorInstance->gridScale, &PL_COLOUR_BLUE );

				PlPopMatrix();
			}
		}
	}
	else
		YnCore_World_Draw( world, currentSector, camera );

#if 0
	static PLVector3 rotation = PLVector3( 0.0f, 0.0f, 0.0f );
	R_DrawAxesPivot( PLVector3( 16.0f, -0.0f, 0.0f ), rotation );
	rotation.x += 0.5f;
	rotation.y += 0.5f;
	rotation.z += 0.5f;
#endif
}

static PLGTexture *colourTexture;
PLGTexture        *YnCore_GetPrimaryColourAttachment( void )
{
	return colourTexture;
}

static PLGTexture *depthTexture;
PLGTexture        *YnCore_GetPrimaryDepthAttachment( void )
{
	return depthTexture;
}

void YR_DrawScene( YNCoreCamera *camera, const YNCoreViewport *viewport )
{
	g_gfxPerfStats.cameraPos = camera->internal->position;

	// We're going to draw into a texture, so set that up first
	YnCore_SetupRenderTarget( &fboBuffer, &colourTexture, &depthTexture, viewport->width, viewport->height );
	PlgBindFrameBuffer( fboBuffer, PLG_FRAMEBUFFER_DRAW );

	PlgClearBuffers( PLG_BUFFER_COLOUR | PLG_BUFFER_DEPTH | PLG_BUFFER_STENCIL );

	PL_GET_CVAR( "r.wireframe", wireframeMode );
	if ( ( camera != NULL && camera->drawMode == YN_CORE_CAMERA_DRAW_MODE_WIREFRAME ) || wireframeMode->b_value )
		PlgEnableGraphicsState( PLG_GFX_STATE_WIREFRAME );

	YR_RenderScene( camera, viewport );

	if ( ( camera != NULL && camera->drawMode == YN_CORE_CAMERA_DRAW_MODE_WIREFRAME ) || wireframeMode->b_value )
		PlgDisableGraphicsState( PLG_GFX_STATE_WIREFRAME );

	PlgBindFrameBuffer( NULL, PLG_FRAMEBUFFER_DRAW );
}
