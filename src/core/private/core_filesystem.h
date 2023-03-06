/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include "node/public/node.h"

const char *FileSystem_GetUserConfigLocation( void );

void FileSystem_SetupConfig( NLNode *root );

void YnCore_FileSystem_MountBaseLocations( void );
void FileSystem_MountLocations( void );
void YnCore_FileSystem_ClearMountedLocations( void );
