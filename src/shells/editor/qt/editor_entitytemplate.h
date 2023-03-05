// Copyright © 2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <QWindow>

namespace os
{
	class EditorEntityTemplateWindow : public QWindow
	{
		Q_OBJECT

	public:
		EditorEntityTemplateWindow( QWindow *parent );
	};
}
