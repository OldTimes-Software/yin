// Copyright © 2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include "editor.h"
#include "editor_entitytemplate.h"

os::EditorEntityTemplateWindow::EditorEntityTemplateWindow( QWindow *parent )
	: QWindow( parent )
{
	setBaseSize( QSize( 640, 480 ) );
	setTitle( "Entity Template Editor" );
}
