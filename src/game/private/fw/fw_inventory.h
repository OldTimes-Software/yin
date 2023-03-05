// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "fw_game.h"

#define FW_INVENTORY_MAX_SLOTS 32

typedef struct FWInventoryItem
{
	char *name;// name of the item
	char *description;
	char *spawnName;
	struct Material *icon;
	unsigned int quantity;
	float weight;
} FWInventoryItem;

typedef struct FWInventory
{
	FWInventoryItem items[ FW_INVENTORY_MAX_SLOTS ];
	unsigned int numItems;
} FWInventory;
