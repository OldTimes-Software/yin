// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plgraphics/plg.h>
#include <plgraphics/plg_texture.h>
#include <plgraphics/plg_mesh.h>
#include <plgraphics/plg_camera.h>

#include "engine_public_renderer.h"

typedef struct RendererStats
{
	PLVector3    cameraPos;
	unsigned int numBatches;
	unsigned int numTriangles;
	unsigned int numFacesDrawn;
	unsigned int numVisiblePortals;
} RendererStats;
extern RendererStats g_gfxPerfStats;

/* todo: introduce container around this */
typedef struct YRSpriteFrame
{
	unsigned int leftOffset;
	unsigned int topOffset;
	PLGTexture  *texture;
} YRSpriteFrame;

typedef struct YRCamera
{
	char              tag[ 32 ];
	bool              active;
	PLGCamera        *internal; /* the camera used for this viewport */
	YRCameraMode      mode;
	YRCameraDrawMode  drawMode;
	struct Actor     *parentActor;
	bool              enablePostProcessing;
	PLLinkedListNode *node;
} YRCamera;

////////////////////////////////////////////////////////////////////

#define YR_MAX_FPS_READINGS 64

typedef struct YRViewport
{
	unsigned int index;
	int          x, y;
	int          width, height;

	YRCamera *camera;

	struct
	{
		double       frameTime, oldTime;
		double       frameReadings[ YR_MAX_FPS_READINGS ];
		unsigned int frameIndex;

		unsigned int numBatches;
		unsigned int numTriangles;
		unsigned int numPolygons;
		unsigned int numPortals;
	} perf;

	void *windowHandle;
} YRViewport;

////////////////////////////////////////////////////////////////////

typedef enum YRLightType
{
	YR_LIGHT_TYPE_OMNI,
	YR_LIGHT_TYPE_SPOT,
	YR_LIGHT_TYPE_SUN,
} YRLightType;

#define YR_MAX_LIGHTS_PER_PASS 8
typedef struct OSLight
{
	YRLightType type;
	PLVector3   position;
	PLVector3   angles;
	PLColourF32 colour;
	float       radius;
} OSLight;
typedef OSLight LightArray[ YR_MAX_LIGHTS_PER_PASS ];

typedef struct YRPassState
{
	bool         mirror;
	unsigned int depth;
} YRPassState;
extern YRPassState rendererState;

#define YR_NUM_SPRITE_ANGLES 8

#include "scenegraph.h"
#include "renderer_material.h"

void YR_Initialize( void );
void YR_Shutdown( void );

void        YR_SetupRenderTarget( PLGFrameBuffer **buffer, PLGTexture **attachment, PLGTexture **depthAttachment, unsigned int w, unsigned int h );
PLGTexture *YR_GetPrimaryColourAttachment( void );
PLGTexture *YR_GetPrimaryDepthAttachment( void );

void YR_SetupDefaultState( const YRViewport *viewport );
void YR_BeginDraw( YRViewport *viewport );
void YR_EndDraw( YRViewport *viewport );

void YR_Set2DViewportSize( int w, int h );
void YR_Get2DViewportSize( int *width, int *height );
void YR_DrawMenu( const YRViewport *viewport );

struct ShaderProgramIndex *RS_GetShaderProgram( const char *name );

void YR_DrawPerspective( YRCamera *camera, const YRViewport *viewport );

void YR_Draw2DQuad( Material *material, int x, int y, int w, int h );
void YR_DrawAxesPivot( PLVector3 position, PLVector3 rotation );

void YR_Sprite_DrawAnimationFrame( YRSpriteFrame *frame, const PLVector3 *position, float spriteAngle );
void YR_Sprite_DrawAnimation( YRSpriteFrame **animation, unsigned int numFrames, unsigned int curFrame, const PLVector3 *position, float angle );

PLGTexture *RT_LoadTexture( const char *path, PLGTextureFilter filterMode );
PLGTexture *YR_Texture_GetFallback( void );

#if 0
typedef struct Texture Texture;
Texture               *Renderer_Texture_Load( const char *path );
void                   Renderer_Texture_Release( Texture *texture );
PLGTexture            *Renderer_Texture_GetInternal( Texture *texture );
#endif

////////////////////////////////////////////////////////////////////

typedef struct YRRenderTarget YRRenderTarget;

YRRenderTarget *YR_RenderTarget_GetByKey( const char *key );
YRRenderTarget *YR_RenderTarget_Create( const char *key, unsigned int width, unsigned int height, unsigned int flags );
void            YR_RenderTarget_Release( YRRenderTarget *renderTarget );
void            YR_RenderTarget_SetSize( YRRenderTarget *renderTarget, unsigned int width, unsigned int height );
PLGTexture     *YR_RenderTarget_GetTextureAttachment( YRRenderTarget *renderTarget );
