/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#define PROTOCOL_VERSION     1
#define PROTOCOL_MESSAGESIZE 4096

typedef struct ProtocolMessageHeader
{
	uint32_t length;
	uint32_t type;
} ProtocolMessageHeader;
