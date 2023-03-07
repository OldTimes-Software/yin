// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_linkedlist.h>
#include <plcore/pl_console.h>

#include <yin/node.h>

/* node structure
 *  string
 *      uint32_t length
 *      char buffer[ length ]
 *
 *  string name
 *  uint8_t type
 *  if type == float: float var
 *  if type == integer: uint32_t var
 *  if type == string: string var
 *  if type == boolean: uint8_t var
 *  if type == object:
 *      uint32_t numChildren
 *      for numChildren
 *          read node
 *
 */

extern int nodeLogLevelPrint;
#define Message( FORMAT, ... ) PlLogMessage( nodeLogLevelPrint, FORMAT, ##__VA_ARGS__ )
extern int nodeLogLevelWarn;
#define Warning( FORMAT, ... ) PlLogMessage( nodeLogLevelWarn, "WARNING: " FORMAT, ##__VA_ARGS__ )

/* upper limits used for the parser */
#define NL_MAX_NAME_LENGTH   256
#define NL_MAX_STRING_LENGTH 256
#define NL_MAX_BOOL_LENGTH   8 /* 0, 1, true, false */
#define NL_MAX_TYPE_LENGTH   16

typedef struct YNNodeVarString
{
	char *buf;
	uint16_t length;
} YNNodeVarString;

typedef struct YNNodeBranch
{
	YNNodeVarString name;
	YNNodePropertyType type;
	YNNodePropertyType childType; /* used for array types */
	YNNodeVarString data;
	YNNodeBranch *parent;

	PLLinkedListNode *linkedListNode;
	PLLinkedList *linkedList;
} YNNodeBranch;

char *YnNode_PreProcessScript( char *buf, size_t *length, bool isHead );
YNNodeBranch *YnNode_PushBackNewBranch( YNNodeBranch *parent, const char *name, YNNodePropertyType propertyType );
