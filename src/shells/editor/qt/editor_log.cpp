//
// Created by hogsy on 05/01/23.
//

#include <plcore/pl_console.h>

#include <QScrollBar>

#include "editor.h"
#include "editor_log.h"

os::EditorLog::EditorLog()
{
	setReadOnly( true );
}

void os::EditorLog::PushMessage( int level, const char *msg, PLColour colour )
{
	appendHtml( QString::asprintf( "<p>"
	                               "[<b style=\"color:rgb(100,0,100)\">%d</b>] %s"
	                               "</p>",
	                               level,
	                               msg ) );
	verticalScrollBar()->setValue( verticalScrollBar()->maximum() );
}
