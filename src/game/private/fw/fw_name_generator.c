// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "game/private/game_private.h"

const char *FW_NameGenerator_Generate( char *buffer, size_t size )
{
	static const char *segments[] = {
	        "aa", "al", "el", "la",
	        "fa", "mo", "re", "ka",
	        "ca", "ma", "fe", "me",
	        "ra", "ke", "ce", "ee",
	        "he", "fo", "ru", "ku",
	        "cu", "eu", "hu", "fu" };

	unsigned int maxSize = ( rand() % size - 1 );
	if ( maxSize < 4 )
		maxSize = 4;

	char *p = buffer;
	for ( size_t i = 0; i < maxSize; i += 2 )
	{
		unsigned int s = rand() % PL_MAX_ARRAY_INDEX( segments );
		*p++           = segments[ s ][ 0 ];
		*p++           = segments[ s ][ 1 ];
	}

	// Ensure the first character is uppercase and null termination.
	buffer[ 0 ]       = ( char ) toupper( buffer[ 0 ] );
	buffer[ maxSize ] = '\0';
	return buffer;
}
