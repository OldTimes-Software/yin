// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_math.h>
#include <plgraphics/plg_camera.h>
#include <plgraphics/plg_texture.h>

#include "../../engine/public/engine_public_renderer.h"

typedef struct GUIVector2
{
	int x, y;
} GUIVector2;

/****************************************
 * Canvas
 ****************************************/

typedef struct GUICanvas GUICanvas;// represents what the GUI draws to

GUICanvas  *GUI_CreateCanvas( int width, int height );
void        GUI_DestroyCanvas( GUICanvas *canvas );
void        GUI_SetCanvasSize( GUICanvas *canvas, int width, int height );
void        GUI_GetCanvasSize( GUICanvas *canvas, int *width, int *height );
PLGTexture *GUI_GetCanvasTexture( GUICanvas *canvas );

/****************************************
 ****************************************/

typedef struct GUIBitmapFont GUIBitmapFont;
typedef struct GUIStyleSheet GUIStyleSheet;

typedef struct GUIPanel GUIPanel;

void GUI_Initialize( void );
void GUI_Shutdown( void );

const GUIStyleSheet *GUI_CacheStyleSheet( const char *path );
void                 GUI_SetStyleSheet( const GUIStyleSheet *styleSheet );
const GUIStyleSheet *GUI_GetActiveStyleSheet( void );

void GUI_Tick( GUIPanel *root );
void GUI_Draw( GUICanvas *canvas, GUIPanel *root );

typedef enum GUIMouseButton
{
	GUI_MOUSE_BUTTON_LEFT,
	GUI_MOUSE_BUTTON_RIGHT,
	GUI_MOUSE_BUTTON_MIDDLE,
	GUI_MAX_MOUSE_BUTTONS
} GUIMouseButton;

void GUI_UpdateMousePosition( int x, int y );
void GUI_UpdateMouseWheel( float x, float y );
void GUI_UpdateMouseButton( GUIMouseButton button, bool isDown );

/****************************************
 * Panel
 ****************************************/

typedef enum GUIPanelBackground
{
	GUI_PANEL_BACKGROUND_NONE,
	GUI_PANEL_BACKGROUND_DEFAULT,
	GUI_PANEL_BACKGROUND_SOLID,
} GUIPanelBackground;

typedef enum GUIPanelBorder
{
	GUI_PANEL_BORDER_NONE,
	GUI_PANEL_BORDER_INSET,
	GUI_PANEL_BORDER_OUTSET,

	GUI_MAX_BORDER_STYLES
} GUIPanelBorder;

GUIPanel *GUI_Panel_Create( GUIPanel *parent, int x, int y, int w, int h, GUIPanelBackground background, GUIPanelBorder border );
void      GUI_Panel_Destroy( GUIPanel *self );

void GUI_Panel_SetStyleSheet( GUIPanel *self, const GUIStyleSheet *styleSheet );

void GUI_Panel_Draw( GUIPanel *self );
void GUI_Panel_DrawBackground( GUIPanel *self );
void GUI_Panel_Tick( GUIPanel *self );

void     GUI_Panel_SetBackgroundColour( GUIPanel *self, const PLColour *colour );
PLColour GUI_Panel_GetBackgroundColour( GUIPanel *self );

void GUI_Panel_SetBorder( GUIPanel *self, GUIPanelBorder border );
void GUI_Panel_SetBackground( GUIPanel *self, GUIPanelBackground background );

GUIPanel *GUI_Panel_GetParent( GUIPanel *self );

void GUI_Panel_GetPosition( GUIPanel *self, int *x, int *y );
void GUI_Panel_GetContentPosition( GUIPanel *self, int *x, int *y );
void GUI_Panel_GetAbsolutePosition( GUIPanel *self, int *x, int *y );

void GUI_Panel_SetPosition( GUIPanel *self, int x, int y );

void GUI_Panel_GetSize( GUIPanel *self, int *w, int *h );
void GUI_Panel_GetContentSize( GUIPanel *self, int *w, int *h );

void GUI_Panel_SetSize( GUIPanel *self, int w, int h );

bool GUI_Panel_IsMouseOver( GUIPanel *self, int mx, int my );

bool GUI_Panel_HandleMouseEvent( GUIPanel *self, int mx, int my, int wheel, int button, bool buttonUp );
bool GUI_Panel_HandleKeyboardEvent( GUIPanel *self, int button, bool buttonUp );

void GUI_Panel_SetVisible( GUIPanel *self, bool flag );

/****************************************
 * Cursor
 ****************************************/

GUIPanel *GUI_Cursor_Create( GUIPanel *parent, int x, int y );
void      GUI_Cursor_Destroy( GUIPanel *self );

/****************************************
 ****************************************/
