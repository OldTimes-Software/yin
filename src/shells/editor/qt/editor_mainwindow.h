// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_filesystem.h>

#include <QTreeView>
#include <QMainWindow>

#include "editor_log.h"

namespace os
{
	class EditorEntityTemplateWindow;

	class EditorMainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		EditorMainWindow();

		void UpdateTitle();

	private:
		static void PushMessageCallback( int level, const char *msg, PLColour colour );

		void GeneratePackage( std::vector< std::string > &files, const char *destination );

		void NewProject();
		void OpenProject();
		void CloseProject();
		void NewPackage();
		void ExtractPackage();
		void About();
		void Quit();

		QMap< const char *, PLFileSystemMount * > mountedLocations;
		PLFileSystemMount *currentMountedLocation;
		bool MountLocation( const char *filename );

		QTreeView *fileTree;
		void UpdateFileTree();

		QMenu *fileMenu;
		QMenu *editMenu;
		QMenu *viewMenu;
		QMenu *toolsMenu;
		QMenu *runMenu;
		QMenu *helpMenu;

		QAction *closeProject;

		EditorLog *logWindow;

		EditorEntityTemplateWindow *entityTemplateWindow{ nullptr };
		void OpenEntityTemplateEditor();
	};
}// namespace os