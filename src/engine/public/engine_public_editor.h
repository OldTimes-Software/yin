// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "common.h"
#include "engine_public_world.h"

PL_EXTERN_C

typedef enum EditorMode
{
	EDITOR_MODE_WORLD,
	EDITOR_MODE_MODEL,
	EDITOR_MODE_MATERIAL,
	EDITOR_MAX_MODES
} EditorMode;

#define EDITOR_MAX_VIEWPORTS      4
#define EDITOR_MAX_VIEW_BOOKMARKS 16

typedef struct EditorField
{
	char name[ 64 ];
	char description[ 128 ];
	CommonDataType type;
	uintptr_t varOffset;
} EditorField;

#define ENTITY_COMPONENT_BEGIN_PROPERTIES() static EditorField x_editorVariables[] = {
#define ENTITY_COMPONENT_END_PROPERTIES() \
	}                                     \
	;                                     \
	static unsigned int x_numEditorVariables = PL_ARRAY_ELEMENTS( x_editorVariables );
#define ENTITY_COMPONENT_PROPERTY( TYPE, VAR, DESC, VARTYPE ) \
	{ #VAR, DESC, VARTYPE, PL_OFFSETOF( TYPE, VAR ) },
#define ENTITY_HOOK_PROPERTIES( CBTABLE )            \
	( CBTABLE ).editorFields    = x_editorVariables; \
	( CBTABLE ).numEditorFields = x_numEditorVariables

typedef struct EditorViewBookmark
{
	char description[ 32 ];
	PLVector3 viewPos;
	PLVector3 viewAngles;
} EditorViewBookmark;

typedef enum EditorGeometryMode
{
	EDITOR_GEOMETRYMODE_VERTEX,
	EDITOR_GEOMETRYMODE_EDGE,
	EDITOR_GEOMETRYMODE_FACE,
	EDITOR_GEOMETRYMODE_BRUSH,

	EDITOR_MAX_GEOMETRYMODES
} EditorGeometryMode;

/* An instance represents literally a unique instance
 * of an editing state, which can have it's own world, model
 * or whatever resource open.
 * */
typedef struct EditorInstance
{
	EditorMode mode;
	union
	{
		struct
		{
			World *world;
		} worldMode;
		struct
		{
			struct PLMModel *model;
		} modelMode;
		struct
		{
			struct Material *material;
			struct PLGMesh *mesh;
		} materialMode;
	};

	bool hideGrid;
	bool useLineGrid;
	PLMatrix4 gridTransform;
	unsigned int gridScale;

	YRViewport *viewports[ EDITOR_MAX_VIEWPORTS ];
	YRCamera *camera;
} EditorInstance;

PL_EXTERN_C_END
