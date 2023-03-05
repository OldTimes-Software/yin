/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_linkedlist.h>
#include <plcore/pl_filesystem.h>

#include "engine/public/engine_public_vm.h"

/*--------------------------
 * VM API
 * */

typedef struct VMProgram VMProgram;

VMProgram *VM_GetProgramByName( const char *programName );
VMProgram *VM_LoadCVM( const char *path, size_t memoryPoolSize );
void VM_Tick( void );
void VM_Initialize( void );
