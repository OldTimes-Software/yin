/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

const char *P_SkipSpaces( const char *buffer );
const char *P_SkipLine( const char *buffer );
const char *P_ReadString( const char *buffer, char *destination, size_t length );
