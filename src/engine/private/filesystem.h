/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include "node/public/node.h"

const char *FileSystem_GetUserConfigLocation( void );

void FileSystem_SetupConfig( NLNode *root );

void FileSystem_MountBaseLocations( void );
void FileSystem_MountLocations( void );
void FileSystem_ClearMountedLocations( void );
