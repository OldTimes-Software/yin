// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "common.h"
#include "core_world.h"

PL_EXTERN_C

typedef enum YNCoreEditorMode
{
	YN_CORE_EDITOR_MODE_WORLD,
	YN_CORE_EDITOR_MODE_MODEL,
	YN_CORE_EDITOR_MODE_MATERIAL,
	YN_CORE_EDITOR_MAX_MODES
} YNCoreEditorMode;

#define YN_CORE_EDITOR_MAX_VIEWPORTS      4
#define YN_CORE_EDITOR_MAX_VIEW_BOOKMARKS 16

typedef struct YNCoreEditorField
{
	char name[ 64 ];
	char description[ 128 ];
	CommonDataType type;
	uintptr_t varOffset;
} YNCoreEditorField;

#define YN_CORE_ENTITY_COMPONENT_BEGIN_PROPERTIES() static YNCoreEditorField x_editorVariables[] = {
#define YN_CORE_ENTITY_COMPONENT_END_PROPERTIES() \
	}                                             \
	;                                             \
	static unsigned int x_numEditorVariables = PL_ARRAY_ELEMENTS( x_editorVariables );
#define YN_CORE_ENTITY_COMPONENT_PROPERTY( TYPE, VAR, DESC, VARTYPE ) \
	{ #VAR, DESC, VARTYPE, PL_OFFSETOF( TYPE, VAR ) },
#define YN_CORE_ENTITY_HOOK_PROPERTIES( CBTABLE )    \
	( CBTABLE ).editorFields    = x_editorVariables; \
	( CBTABLE ).numEditorFields = x_numEditorVariables

typedef struct YNCoreEditorViewBookmark
{
	char description[ 32 ];
	PLVector3 viewPos;
	PLVector3 viewAngles;
} YNCoreEditorViewBookmark;

typedef enum YNCoreEditorGeometryMode
{
	EDITOR_GEOMETRYMODE_VERTEX,
	EDITOR_GEOMETRYMODE_EDGE,
	EDITOR_GEOMETRYMODE_FACE,
	EDITOR_GEOMETRYMODE_BRUSH,

	EDITOR_MAX_GEOMETRYMODES
} YNCoreEditorGeometryMode;

/* An instance represents literally a unique instance
 * of an editing state, which can have it's own world, model
 * or whatever resource open.
 * */
typedef struct YNCoreEditorInstance
{
	YNCoreEditorMode mode;
	union
	{
		struct
		{
			YNCoreWorld *world;
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

	YNCoreViewport *viewports[ YN_CORE_EDITOR_MAX_VIEWPORTS ];
	YNCoreCamera *camera;
} YNCoreEditorInstance;

PL_EXTERN_C_END
