// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "editor_viewport_frame.h"
#include "core_renderer.h"

#include <plgraphics/plg.h>
#include <plgraphics/plg_camera.h>

#include <FXGLCanvas.h>
#include <FXGLVisual.h>

FXGLCanvas *EditorViewportFrame::displayList_ = nullptr;

FXDEFMAP( EditorViewportFrame )
editorViewportMap[] = {
        FXMAPFUNC( SEL_CHORE, EditorViewportFrame::ID_CHORE, EditorViewportFrame::OnChore ),
        FXMAPFUNC( SEL_MOTION, EditorViewportFrame::ID_CANVAS, EditorViewportFrame::OnMotion ),
};

FXIMPLEMENT( EditorViewportFrame, FXVerticalFrame, editorViewportMap, ARRAYNUMBER( editorViewportMap ) )

EditorViewportFrame::EditorViewportFrame( FXComposite *composite, FXGLVisual *visual, YRCameraMode viewMode )
    : FXVerticalFrame( composite, FRAME_NORMAL | LAYOUT_FILL | LAYOUT_TOP | LAYOUT_LEFT,
                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0 )
{
	engineViewportHandle = YinCore_Viewport_Create( 0, 0, 800, 600, this );

	//engineViewportHandle.viewMode = viewMode;
	//engineViewportHandle.drawMode = ( viewMode == YR_CAMERA_MODE_PERSPECTIVE ) ? YR_CAMERA_DRAW_MODE_TEXTURED : YR_CAMERA_DRAW_MODE_WIREFRAME;

#if 1
	toolBar_ = new FXToolBar( this, FRAME_RAISED | LAYOUT_DOCK_SAME | LAYOUT_SIDE_TOP | LAYOUT_FILL_X );
	new FXButton( toolBar_, FXString::null, os::editor::LoadFXIcon( getApp(), "resources/eye.gif" ) );
	new FXButton( toolBar_, FXString::null, os::editor::LoadFXIcon( getApp(), "resources/top.gif" ) );
	new FXButton( toolBar_, FXString::null, os::editor::LoadFXIcon( getApp(), "resources/left.gif" ) );
	new FXButton( toolBar_, FXString::null, os::editor::LoadFXIcon( getApp(), "resources/front.gif" ) );
	new FXVerticalSeparator( toolBar_ );
	new FXButton( toolBar_, FXString::null, os::editor::LoadFXIcon( getApp(), "resources/wireframe.gif" ) );
	new FXButton( toolBar_, FXString::null, os::editor::LoadFXIcon( getApp(), "resources/solid.gif" ) );
	new FXButton( toolBar_, FXString::null, os::editor::LoadFXIcon( getApp(), "resources/textured.gif" ) );
	new FXVerticalSeparator( toolBar_ );
	new FXTextField( toolBar_, 4, &forwardSpeedTarget_, FXDataTarget::ID_VALUE, TEXTFIELD_LIMITED | TEXTFIELD_INTEGER | FRAME_NORMAL );
	new FXTextField( toolBar_, 4, &turnSpeedTarget_, FXDataTarget::ID_VALUE, TEXTFIELD_LIMITED | TEXTFIELD_INTEGER | FRAME_NORMAL );
	new FXVerticalSeparator( toolBar_ );
	new FXButton( toolBar_, FXString::null, os::editor::LoadFXIcon( getApp(), "resources/popout.gif" ) );
#endif

	visual_ = visual;
	if ( displayList_ == nullptr )
	{
		canvas_      = new FXGLCanvas( this, visual_, this, ID_CANVAS, LAYOUT_FILL );
		displayList_ = canvas_;
	}
	else
		canvas_ = new FXGLCanvas( this, visual_, displayList_, this, ID_CANVAS, LAYOUT_FILL );

	getApp()->addChore( this, ID_CHORE );
}

EditorViewportFrame::~EditorViewportFrame()
{
	getApp()->removeChore( this, ID_CHORE );

	YinCore_Viewport_Destroy( engineViewportHandle );

	canvas_->makeNonCurrent();
	delete canvas_;
}

void EditorViewportFrame::create()
{
	FXVerticalFrame::create();

	show();
	enable();

	canvas_->makeCurrent();
}

void EditorViewportFrame::Draw()
{
	canvas_->makeCurrent();

	int w = canvas_->getWidth();
	if ( w < 2 )
		w = 2;

	int h = canvas_->getHeight();
	if ( h < 2 )
		h = 2;

	PlgSetViewport( 0, 0, w, h );

	if ( YinCore_IsEngineRunning() )
	{
		YinCore_Viewport_SetSize( engineViewportHandle, w, h );
		YinCore_RenderFrame( engineViewportHandle );
	}
	else
	{
		PlgSetClearColour( PLColourRGB( 100, 0, 0 ) );
		PlgClearBuffers( PLG_BUFFER_COLOUR | PLG_BUFFER_DEPTH );
	}

	if ( visual_->isDoubleBuffer() )
		canvas_->swapBuffers();
}

long EditorViewportFrame::OnChore( FXObject *, FXSelector, void * )
{
	Draw();

	getApp()->addChore( this, ID_CHORE );
	return 1;
}

long EditorViewportFrame::OnMotion( FXObject *, FXSelector, void *ptr )
{
	auto *event = ( FXEvent * ) ptr;
	int const x = event->win_x;
	int const y = event->win_y;

	YinCore_HandleMouseMotionEvent( x, y );

	return 0;
}
