// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "engine_public_editor.h"

PL_EXTERN_C

void Editor_Initialize( void );
void Editor_Shutdown( void );
void Editor_Tick( void );
void Editor_Draw( const YRViewport *viewport );

void Editor_Commands_Register( void );

void Editor_MaterialSelector_Initialize( void );
void Editor_MaterialSelector_Shutdown( void );
void Editor_MaterialSelector_Draw( const YRViewport *viewport );

EditorInstance *Editor_GetCurrentInstance( void );
void            Editor_SetCurrentInstance( EditorInstance *instance );
EditorInstance *Editor_CreateInstance( EditorMode mode );
void            Editor_DestroyInstance( EditorInstance *instance );

PL_EXTERN_C_END
