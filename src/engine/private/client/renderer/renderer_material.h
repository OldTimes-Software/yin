/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#define MAX_MATERIAL_PASSES    4
#define MAX_MATERIAL_VARIABLES 64

/* built-in variable types */
typedef enum MaterialBuiltinVar
{
	MATERIAL_BUILTIN_INVALID = -1,
	MATERIAL_BUILTIN_TIME,
	MATERIAL_BUILTIN_DEPTH,
	MATERIAL_BUILTIN_VIEWPORT_SIZE,

	MAX_MATERIAL_BUILTINS
} MaterialBuiltinVar;

typedef struct Material Material;

#define MATERIAL_VAR_NAME_LENGTH   64
#define MATERIAL_VAR_STRING_LENGTH 256

typedef enum MaterialVariableType
{
	MATERIAL_VAR_INVALID,

	MATERIAL_VAR_FLOAT,
	MATERIAL_VAR_INT,
	MATERIAL_VAR_UINT,
	MATERIAL_VAR_BOOL,
	MATERIAL_VAR_DOUBLE,

	MATERIAL_VAR_VEC2,
	MATERIAL_VAR_VEC3,
	MATERIAL_VAR_VEC4,

	MATERIAL_VAR_MAT3,
	MATERIAL_VAR_MAT4,

	/* special types */
	MATERIAL_VAR_STRING,
	MATERIAL_VAR_TEXTURE,
	MATERIAL_VAR_BUILTIN,
	MATERIAL_VAR_RENDERTARGET,

	MAX_MATERIAL_VAR_TYPES
} MaterialVariableType;

/**
 * Hints for standard material variables, so
 * that we can toggle their state.
 */
typedef enum MaterialVariableHint
{
	RM_VAR_HINT_DIFFUSE,
	RM_VAR_HINT_NORMAL,
	RM_VAR_HINT_SPECULAR,
} MaterialVariableHint;

typedef union MaterialVariableData
{
	float  f32;
	double f64;

	bool boolean;

	int32_t  i32;
	uint32_t ui32;

	PLVector2 vec2;
	PLVector3 vec3;
	PLVector4 vec4;

	PLMatrix3 mat3;
	PLMatrix4 mat4;

	char str[ MATERIAL_VAR_STRING_LENGTH ];

	void *userPtr;
} MaterialVariableData;

typedef struct MaterialVariable
{
	int                  programSlot;
	char                 name[ MATERIAL_VAR_NAME_LENGTH ];
	MaterialVariableType type;
	MaterialVariableData data;
	MaterialVariableHint hint;
} MaterialVariable;

typedef struct MaterialPass
{
	PLGShaderProgram *program;
	PLGTextureFilter  textureFilter;
	PLGBlend          blendMode[ 2 ];
	MaterialVariable  variables[ MAX_MATERIAL_VARIABLES ];
	unsigned int      numVariables;

	bool depthTest;
	int  cullMode;
} MaterialPass;

#define RS_PROGRAM_NAME_LENGTH 64

enum
{
	RS_SHADER_DEFAULT,
	RS_SHADER_LIGHTING_PASS,
	RS_SHADER_DEFAULT_VERTEX,
	RS_SHADER_DEFAULT_ALPHA,

	RS_MAX_DEFAULT_SHADERS
};
extern PLGShaderProgram *defaultShaderPrograms[ RS_MAX_DEFAULT_SHADERS ];

typedef struct ShaderProgramIndex
{
	char path[ PL_SYSTEM_MAX_PATH ];
	char shaderPaths[ PLG_MAX_SHADER_TYPES ][ PL_SYSTEM_MAX_PATH ];
	char internalName[ RS_PROGRAM_NAME_LENGTH ];

	MaterialPass defaultPass;

	PLGShaderProgram        *internalPtr;
	struct PLLinkedListNode *node;
} ShaderProgramIndex;

void RM_ParseMaterialPass( struct NLNode *root, MaterialPass *materialPass );

void YR_Material_Initialize( void );
void RM_ShutdownMaterialSystem( void );

PLGTexture *RM_GetPreviewTexture( Material *material );

Material *RM_GetFallbackMaterial( void );
