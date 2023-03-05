// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

// aaaahhhhh!!!
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QSplitter>
#include <QToolBar>
#include <QToolButton>
#include <QTextEdit>
#include <QOpenGLWidget>
#include <QTreeView>
#include <QProgressBar>
#include <QProgressDialog>
#include <QFileSystemModel>
#include <QFileIconProvider>

#include "editor.h"
#include "editor_mainwindow.h"
#include "editor_entitytemplate.h"

os::EditorMainWindow::EditorMainWindow()
{
	QFileSystemModel model;
	QFileIconProvider iconProvider;
	model.setIconProvider( &iconProvider );
	model.setRootPath( "" );

	fileTree = new QTreeView( this );
	fileTree->resize( 128, fileTree->height() );
	fileTree->setModel( &model );

	auto tabCollection = new QTabWidget;
	tabCollection->addTab( new QTextEdit, "Thingy" );

	logWindow = new EditorLog;

	auto splitter = new QSplitter( this );
	splitter->setContentsMargins( 5, 5, 5, 5 );
	splitter->addWidget( fileTree );
	//splitter->addWidget( tabCollection );
	//splitter->addWidget( logWindow );

	QOpenGLWidget *glWidget = new QOpenGLWidget;
	splitter->addWidget( glWidget );
	setCentralWidget( splitter );
	//splitter->hide();

	statusBar()->showMessage( tr( "Ready" ) );

#if 0
	QToolBar *toolBar = new QToolBar;
	toolBar->addAction( QPixmap( QString( cachedPaths[ PATH_RESOURCES ] ) + "/new.gif" ), "New World" );
	toolBar->addAction( QPixmap( QString( cachedPaths[ PATH_RESOURCES ] ) + "/open.gif" ), "Open World" );
	toolBar->addAction( QPixmap( QString( cachedPaths[ PATH_RESOURCES ] ) + "/save.gif" ), "Save World" );
	toolBar->addSeparator();
	toolBar->addAction( QPixmap( QString( cachedPaths[ PATH_RESOURCES ] ) + "/material_editor.gif" ), "Material Editor" );
	toolBar->addAction( QPixmap( QString( cachedPaths[ PATH_RESOURCES ] ) + "/entity_editor.gif" ), "Entity Editor" );
	toolBar->addSeparator();
	toolBar->addAction( QPixmap( QString( cachedPaths[ PATH_RESOURCES ] ) + "/play_controller.gif" ), "Play" );
	addToolBar( toolBar );
#endif

	// Create our menus

	QAction *action;

	fileMenu = menuBar()->addMenu( tr( "&File" ) );
	action   = new QAction( tr( "&New Project" ), this );
	action->setShortcuts( QKeySequence::New );
	action->setStatusTip( tr( "Create a new project" ) );
	connect( action, &QAction::triggered, this, &EditorMainWindow::NewProject );
	fileMenu->addAction( action );
	action = new QAction( tr( "&Open Project" ), this );
	action->setShortcut( QKeySequence::Open );
	action->setStatusTip( tr( "Open an existing project" ) );
	connect( action, &QAction::triggered, this, &EditorMainWindow::OpenProject );
	fileMenu->addAction( action );
	action = new QAction( tr( "&Close Project" ), this );
	action->setShortcut( QKeySequence::Open );
	action->setStatusTip( tr( "Close the current project" ) );
	connect( action, &QAction::triggered, this, &EditorMainWindow::CloseProject );
	fileMenu->addAction( action );

	fileMenu->addSeparator();
	action = new QAction( tr( "Package &Project" ), this );
	action->setStatusTip( tr( "Create a new package from project" ) );
	connect( action, &QAction::triggered, this, &EditorMainWindow::NewPackage );
	fileMenu->addAction( action );
	action = new QAction( tr( "&Extract Package" ), this );
	action->setStatusTip( tr( "Extract a package" ) );
	connect( action, &QAction::triggered, this, &EditorMainWindow::ExtractPackage );
	fileMenu->addAction( action );
	fileMenu->addSeparator();
	action = new QAction( tr( "&Quit" ), this );
	action->setShortcut( QKeySequence::Quit );
	action->setStatusTip( tr( "Quit the application" ) );
	connect( action, &QAction::triggered, this, &EditorMainWindow::Quit );
	fileMenu->addAction( action );

	//editMenu = menuBar()->addMenu( tr( "&Edit" ) );

	//viewMenu = menuBar()->addMenu( tr( "&View" ) );

	toolsMenu = menuBar()->addMenu( tr( "&Tools" ) );
	action    = new QAction( tr( "Entity Editor" ), this );
	action->setStatusTip( tr( "Create a new package" ) );
	connect( action, &QAction::triggered, this, &EditorMainWindow::OpenEntityTemplateEditor );
	toolsMenu->addAction( action );
	action = new QAction( tr( "Material Editor" ), this );
	action->setStatusTip( tr( "Create a new package" ) );
	connect( action, &QAction::triggered, this, &EditorMainWindow::NewPackage );
	toolsMenu->addAction( action );

	helpMenu = menuBar()->addMenu( tr( "&Help" ) );
	action   = new QAction( tr( "&About" ), this );
	action->setStatusTip( tr( "Display information about the application" ) );
	connect( action, &QAction::triggered, this, &EditorMainWindow::About );
	helpMenu->addAction( action );

	resize( 1280, 720 );

	PlSetConsoleOutputCallback( &EditorMainWindow::PushMessageCallback );
}

void os::EditorMainWindow::PushMessageCallback( int level, const char *msg, PLColour colour )
{
	mainWindow->logWindow->PushMessage( level, msg, colour );
}

void os::EditorMainWindow::GeneratePackage( std::vector< std::string > &files, const char *destination )
{
	FILE *out = fopen( destination, "wb" );
	if ( out == nullptr )
	{
		QMessageBox::warning( this, "Error", "Failed to write file!" );
		return;
	}

	QProgressDialog progress( "Generating package...", QString(), 0, files.size(), this );
	progress.setWindowModality( Qt::WindowModal );

	Common_Pkg_WriteHeader( out, files.size() );

	for ( unsigned int i = 0; i < files.size(); ++i )
	{
		const char *filename = files[ i ].c_str();
		PLFile *file         = PlOpenFile( filename, true );
		if ( file == nullptr )
		{
			QMessageBox::warning( this, "Error", QString( "Failed to open file: " ) + filename );
			break;
		}

		Common_Pkg_AddData( out, filename, PlGetFileData( file ), PlGetFileSize( file ) );

		progress.setValue( i );

		PlCloseFile( file );
	}
	progress.setValue( files.size() );

	fclose( out );

	// And automatically mount it
	MountLocation( destination );
}

void os::EditorMainWindow::NewProject()
{
	QDialog dialog( this );
	dialog.setWindowTitle( tr( "Enter Project Name" ) );

	QVBoxLayout boxLayout( &dialog );

	QLineEdit textField( &dialog );
	textField.setFixedWidth( 256 );
	boxLayout.addWidget( &textField );

	QDialogButtonBox buttonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this );
	connect( &buttonBox, SIGNAL( accepted() ), &dialog, SLOT( accept() ) );
	connect( &buttonBox, SIGNAL( rejected() ), &dialog, SLOT( reject() ) );
	boxLayout.addWidget( &buttonBox );

	dialog.setLayout( &boxLayout );

	dialog.show();
	if ( dialog.exec() != QDialog::Accepted )
	{
		return;
	}

	// now create the new project in our projects location
	const char *projectName = textField.text().toUtf8().data();
	if ( *projectName == '\0' )
	{
		QMessageBox::warning( this, "Error", tr( "Please enter a valid project name!" ) );
		return;
	}

	// create a simplfied variant of the name

	char folderName[ 64 ];
	PL_ZERO_( folderName );

	// ensure it's not too long
	size_t pnl = strlen( projectName );
	if ( pnl >= sizeof( folderName ) )
	{
		pnl = sizeof( folderName ) - 1;
	}

	// and now generate it
	char *c = folderName;
	for ( unsigned int i = 0; i < pnl; ++i )
	{
		if ( isalnum( projectName[ i ] ) == 0 && projectName[ i ] != ' ' )
			continue;

		if ( projectName[ i ] == ' ' ) { *c = '_'; }
		else { *c = tolower( projectName[ i ] ); }
		c++;
	}

	assert( folderName[ 0 ] != '\0' );
	if ( folderName[ 0 ] == '\0' )
	{
		QMessageBox::warning( this, "Error", tr( "Please enter a valid project name!" ) );
		return;
	}

	// now create the project
	if ( !os::CreateProject( projectName, folderName ) )
	{
		return;
	}

	os::OpenProject( folderName );

	UpdateTitle();
}

void os::EditorMainWindow::OpenProject()
{
	QFileDialog folderDialog( this, "Select a Folder" );
	folderDialog.setDirectory( os::cachedPaths[ os::PATH_PROJECTS ] );
	folderDialog.setAcceptMode( QFileDialog::AcceptOpen );
	folderDialog.setFileMode( QFileDialog::Directory );
	folderDialog.show();

	if ( folderDialog.exec() != QDialog::Accepted )
	{
		return;
	}

	const char *selectedFolder = folderDialog.selectedFiles()[ 0 ].toUtf8().data();
	if ( !os::OpenProject( selectedFolder ) )
	{
		return;
	}

	UpdateTitle();
}

void os::EditorMainWindow::CloseProject()
{
	UpdateTitle();
}

void os::EditorMainWindow::NewPackage()
{
	if ( os::editorProject.config == nullptr )
	{
		QMessageBox::warning( this, "Warning", tr( "No active project! Please open a project first." ) );
		return;
	}

	QFileDialog fileDialog( this, "Select Destination", QString(), "*.pkg" );
	fileDialog.setAcceptMode( QFileDialog::AcceptSave );
	fileDialog.setFileMode( QFileDialog::AnyFile );
	fileDialog.show();
	if ( fileDialog.exec() == QDialog::Accepted )
	{
		// Build up file tree
		std::vector< std::string > files;
		PlScanDirectory(
		        os::editorProject.path, NULL, []( const char *filename, void *user )
		        { ( ( std::vector< std::string > * ) user )->push_back( filename ); },
		        true, &files );

		const char *selectedFile = fileDialog.selectedFiles()[ 0 ].toUtf8().data();
		GeneratePackage( files, selectedFile );
	}
}

void os::EditorMainWindow::ExtractPackage()
{
	QFileDialog fileDialog( this, "Select a Package", QString(), "*.pkg" );
	fileDialog.setAcceptMode( QFileDialog::AcceptOpen );
	fileDialog.setFileMode( QFileDialog::ExistingFile );
	fileDialog.show();

	if ( fileDialog.exec() == QDialog::Accepted )
	{
		QString const selectedFile = fileDialog.selectedFiles()[ 0 ];

		// Now for the destination
		QFileDialog folderDialog( this, "Select a Destination" );
		folderDialog.setAcceptMode( QFileDialog::AcceptOpen );
		folderDialog.setFileMode( QFileDialog::Directory );
		folderDialog.show();

		if ( folderDialog.exec() != QDialog::Accepted )
		{
			return;
		}
	}
}

void os::EditorMainWindow::About()
{
	QString const versionInfo = "v" + QString::number( EDITOR_VERSION_MAJOR ) +
	                            "." + QString::number( EDITOR_VERSION_MINOR ) +
	                            "." + QString::number( EDITOR_VERSION_PATCH );
	QString const buildInfo = "b" + QString( GIT_COMMIT_COUNT );

	QMessageBox about;
	about.setWindowTitle( "About " EDITOR_BASE_TITLE );
	about.setText( "<b>" EDITOR_BASE_TITLE "</b> is an editor frontend designed "
	               "for the Yin 3D Game Engine, written by <a>Mark Sowden</a>." );
	about.setInformativeText(
	        "Copyright \u00A9 2020-2023 OldTimes Software, Mark E Sowden"
	        "<hr><i>" +
	        versionInfo + " " + buildInfo + " (" __DATE__ ")</i>" );
	about.setStandardButtons( QMessageBox::Ok );
	about.setIconPixmap( QPixmap( QString( cachedPaths[ PATH_RESOURCES ] ) + "/logo.png" ) );
	about.show();
	about.exec();
}

void os::EditorMainWindow::Quit()
{
	//TODO: need to shut everything down - make sure there aren't any open documents etc.
	exit( 0 );
}

void os::EditorMainWindow::UpdateTitle()
{
	char projectName[ 64 ];
	if ( os::editorProject.name == nullptr )
	{
		strcpy( projectName, "No active project" );
	}
	else
	{
		snprintf( projectName, sizeof( projectName ), "%s", os::editorProject.name );
	}

	setWindowTitle( QString( EDITOR_BASE_TITLE ) + " - " + projectName );
}

////////////////////////////////////////////////////////////////////

bool os::EditorMainWindow::MountLocation( const char *filename )
{
	PLFileSystemMount *mount = PlMountLocalLocation( filename );
	if ( mount == nullptr )
	{
		QMessageBox::warning( this, "Error", QString( "Failed to mount the specified location!\n" ) + QString( PlGetError() ) );
		return false;
	}

	const char *c = strrchr( filename, '/' ) + 1;
	if ( c == nullptr )
	{
		c = filename;
	}

	mountedLocations.insert( c, mount );
	return true;
}

////////////////////////////////////////////////////////////////////

void os::EditorMainWindow::UpdateFileTree()
{
}

void os::EditorMainWindow::OpenEntityTemplateEditor()
{
	if ( entityTemplateWindow == nullptr )
	{
		//entityTemplateWindow = new EditorEntityTemplateWindow( this );
		entityTemplateWindow->show();
		return;
	}

	entityTemplateWindow->show();
}
