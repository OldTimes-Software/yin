// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_console.h>

#include "gui_private.h"

/****************************************
 * GUI DRAW API
 ****************************************/

typedef struct GUIDrawBatch
{
	PLGMesh    *mesh;
	PLGTexture *texture;
	bool        usedThisFrame;
} GUIDrawBatch;

/****************************************
 * Canvas
 ****************************************/

typedef struct GUICanvas
{
	PLGFrameBuffer *buffer;
	PLGTexture     *texture;
	bool            filter;
	int             width;
	int             height;
} GUICanvas;

GUICanvas *GUI_CreateCanvas( int width, int height )
{
	GUICanvas *canvas = PL_NEW( GUICanvas );
	canvas->width     = width;
	canvas->height    = height;
	canvas->buffer    = PlgCreateFrameBuffer( canvas->width, canvas->height, PLG_BUFFER_COLOUR | PLG_BUFFER_DEPTH );
	canvas->texture   = PlgGetFrameBufferTextureAttachment( canvas->buffer, PLG_BUFFER_COLOUR, PLG_TEXTURE_FILTER_LINEAR );
	return canvas;
}

void GUI_DestroyCanvas( GUICanvas *canvas )
{
	if ( canvas == NULL )
	{
		return;
	}
	PlgDestroyTexture( canvas->texture );
	PlgDestroyFrameBuffer( canvas->buffer );
	PL_DELETE( canvas );
}

void GUI_SetCanvasSize( GUICanvas *canvas, int width, int height )
{
	if ( canvas->width == width && canvas->height == height )
	{
		return;
	}

	PlgSetFrameBufferSize( canvas->buffer, canvas->width, canvas->height );
	if ( canvas->texture != NULL )
	{
		PlgDestroyTexture( canvas->texture );
		canvas->texture = PlgGetFrameBufferTextureAttachment( canvas->buffer, PLG_BUFFER_COLOUR, PLG_TEXTURE_FILTER_LINEAR );
	}
}

void GUI_GetCanvasSize( GUICanvas *canvas, int *width, int *height )
{
	if ( width != NULL )
	{
		*width = canvas->width;
	}
	if ( height != NULL )
	{
		*height = canvas->height;
	}
}

PLGTexture *GUI_GetCanvasTexture( GUICanvas *canvas )
{
	return canvas->texture;
}

/****************************************
 ****************************************/

static PLGCamera *camera;

static PLLinkedList *batches;

static bool hasBegun = false;

void GUI_Draw_Initialize( void )
{
	batches = PlCreateLinkedList();

	camera       = PlgCreateCamera();
	camera->mode = PLG_CAMERA_MODE_ORTHOGRAPHIC;
	camera->near = 0.0f;
	camera->far  = 1000.0f;
}

void GUI_Draw_Shutdown( void )
{
}

PLGMesh *GUI_Draw_GetBatchQueueMesh( PLGTexture *texture )
{
	PLLinkedListNode *node = PlGetFirstNode( batches );
	while ( node != NULL )
	{
		GUIDrawBatch *drawBatch = PlGetLinkedListNodeUserData( node );
		if ( drawBatch->texture == texture )
		{
			return drawBatch->mesh;
		}

		node = PlGetNextLinkedListNode( node );
	}

	// Texture isn't in the queue, so create a new batch request
	GUIDrawBatch *drawBatch = PL_NEW( GUIDrawBatch );
	drawBatch->mesh         = PlgCreateMesh( PLG_MESH_TRIANGLES, PLG_DRAW_DYNAMIC, 256, 256 );
	drawBatch->texture      = texture;
	PlInsertLinkedListNode( batches, drawBatch );
	return drawBatch->mesh;
}

static void CleanupBatchQueue( void )
{
	PLLinkedListNode *node = PlGetFirstNode( batches );
	while ( node != NULL )
	{
		GUIDrawBatch *drawBatch = PlGetLinkedListNodeUserData( node );
		if ( drawBatch->mesh->num_triangles == 0 )
		{
			PlgDestroyMesh( drawBatch->mesh );

			PL_DELETE( drawBatch );

			PLLinkedListNode *prevNode = node;
			node                       = PlGetNextLinkedListNode( node );
			PlDestroyLinkedListNode( prevNode );
			continue;
		}

		PlgClearMesh( drawBatch->mesh );
		node = PlGetNextLinkedListNode( node );
	}

	guiState.lastNumTriangles = guiState.numTriangles;
	guiState.numTriangles     = 0;
	guiState.lastNumBatches   = guiState.numBatches;
	guiState.numBatches       = 0;
}

void GUI_Draw( GUICanvas *canvas, GUIPanel *root )
{
	// save old state
	int ox, oy, ow, oh;
	PlgGetViewport( &ox, &oy, &ow, &oh );
	PLMatrix4 oldViewMatrix = PlgGetViewMatrix();

	PlgSetViewport( 0, 0, canvas->width, canvas->height );

	CleanupBatchQueue();

	PlgBindFrameBuffer( canvas->buffer, PLG_FRAMEBUFFER_DRAW );

	PlgSetupCamera( camera );
	PlgClearBuffers( PLG_BUFFER_COLOUR | PLG_BUFFER_DEPTH );

	PlMatrixMode( PL_MODELVIEW_MATRIX );
	PlPushMatrix();

	PlLoadIdentityMatrix();

	GUI_Panel_Draw( root );

	PLGShaderProgram *program = PlgGetCurrentShaderProgram();
	PlgSetShaderUniformValue( program, "pl_model", PlGetMatrix( PL_MODELVIEW_MATRIX ), false );

	PLLinkedListNode *node = PlGetFirstNode( batches );
	while ( node != NULL )
	{
		GUIDrawBatch *drawBatch = PlGetLinkedListNodeUserData( node );
		PlgSetTexture( drawBatch->texture, 0 );
		PlgUploadMesh( drawBatch->mesh );
		PlgDrawMesh( drawBatch->mesh );

		guiState.numTriangles += drawBatch->mesh->num_triangles;
		guiState.numBatches++;

		node = PlGetNextLinkedListNode( node );
	}

	PlPopMatrix();

	PlgBindFrameBuffer( NULL, PLG_FRAMEBUFFER_DRAW );

	PlgSetTexture( NULL, 0 );

	// restore
	PlgSetViewMatrix( &oldViewMatrix );
	PlgSetViewport( ox, oy, ow, oh );

	//printf( "%d tris, %d batches\n", guiState.numTriangles, guiState.numBatches );
}

void GUI_Draw_FilledRectangle( PLGMesh *mesh, int x, int y, int w, int h, int z, const PLColour *colour )
{
	unsigned int vertices[] = {
	        PlgAddMeshVertex( mesh, &PLVector3( x, y, z ), &pl_vecOrigin3, colour, &pl_vecOrigin2 ),
	        PlgAddMeshVertex( mesh, &PLVector3( x, y + h, z ), &pl_vecOrigin3, colour, &pl_vecOrigin2 ),
	        PlgAddMeshVertex( mesh, &PLVector3( x + w, y, z ), &pl_vecOrigin3, colour, &pl_vecOrigin2 ),
	        PlgAddMeshVertex( mesh, &PLVector3( x + w, y + h, z ), &pl_vecOrigin3, colour, &pl_vecOrigin2 ),
	};

	PlgAddMeshTriangle( mesh, vertices[ 0 ], vertices[ 1 ], vertices[ 2 ] );
	PlgAddMeshTriangle( mesh, vertices[ 1 ], vertices[ 3 ], vertices[ 2 ] );
}

/**
 * Similar to Draw_FilledRectangle, only more explicit for the frame coords.
 */
void GUI_Draw_Quad( PLGMesh *mesh, GUIVector2 tl, GUIVector2 tr, GUIVector2 ll, GUIVector2 lr, int z, const PLColourF32 *colour )
{
	// todo: drawing API should take floating-point colours!
	PLColour bColour = PlColourF32ToU8( colour );

	unsigned int vertices[] = {
	        PlgAddMeshVertex( mesh, &PLVector3( tl.x, tl.y, z ), &pl_vecOrigin3, &bColour, &pl_vecOrigin2 ),
	        PlgAddMeshVertex( mesh, &PLVector3( tr.x, tr.y, z ), &pl_vecOrigin3, &bColour, &pl_vecOrigin2 ),
	        PlgAddMeshVertex( mesh, &PLVector3( ll.x, ll.y, z ), &pl_vecOrigin3, &bColour, &pl_vecOrigin2 ),
	        PlgAddMeshVertex( mesh, &PLVector3( lr.x, lr.y, z ), &pl_vecOrigin3, &bColour, &pl_vecOrigin2 ),
	};

	PlgAddMeshTriangle( mesh, vertices[ 0 ], vertices[ 1 ], vertices[ 2 ] );
	PlgAddMeshTriangle( mesh, vertices[ 1 ], vertices[ 3 ], vertices[ 2 ] );
}
