// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <yin/core_editor.h>

PL_EXTERN_C

void Editor_Initialize( void );
void YnCore_ShutdownEditor( void );
void Editor_Tick( void );
void Editor_Draw( const YNCoreViewport *viewport );

void Editor_Commands_Register( void );

void Editor_MaterialSelector_Initialize( void );
void Editor_MaterialSelector_Shutdown( void );
void Editor_MaterialSelector_Draw( const YNCoreViewport *viewport );

YNCoreEditorInstance *Editor_GetCurrentInstance( void );
void            Editor_SetCurrentInstance( YNCoreEditorInstance *instance );
YNCoreEditorInstance *Editor_CreateInstance( YNCoreEditorMode mode );
void            Editor_DestroyInstance( YNCoreEditorInstance *instance );

PL_EXTERN_C_END
