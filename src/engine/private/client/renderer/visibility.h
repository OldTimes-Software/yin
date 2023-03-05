// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

PLLinkedList *VIS_GetVisibleFaces( YRCamera *camera, PLLinkedList *faces );
PLLinkedList *VIS_GetVisiblePortals( YRCamera *camera, PLLinkedList *faces );
