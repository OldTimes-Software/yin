// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "fw_component_stats.h"

static void FW_Component_Stats_Spawn( YNCoreEntityComponent *self )
{
}

const YNCoreEntityComponentCallbackTable *FW_Component_Stats_GetCallbackTable( void )
{
	static YNCoreEntityComponentCallbackTable callbackTable;
	PL_ZERO_( callbackTable );
	callbackTable.spawnFunction = FW_Component_Stats_Spawn;

	return &callbackTable;
}
