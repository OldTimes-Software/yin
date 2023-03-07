// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include "kernel/plcore/include/plcore/pl_linkedlist.h"
#include "kernel/plcore/include/plcore/pl_parse.h"

#include "node_private.h"

//#define DEBUG_PARSER_MESSAGES
#if !defined( NDEBUG ) && defined( DEBUG_PARSER_MESSAGES )
#	define DEBUG_PARSER( FORMAT, ... ) Message( "PARSE: " FORMAT, ##__VA_ARGS__ )
#else
#	define DEBUG_PARSER( FORMAT, ... )
#endif

static void SkipToNextToken( const char **buf, unsigned int *line )
{
	DEBUG_PARSER( "START:%s\n", ( *buf ) );
	while ( *( *buf ) == ' ' || *( *buf ) == '\t' || *( *buf ) == '\n' || *( *buf ) == '\r' )
	{
		if ( *( *buf ) == '\n' ) line++;
		( *buf )++;
	}
	DEBUG_PARSER( "END:%s\n", ( *buf ) );
}

static const char *ParseToken( const char **buf, char *token, size_t size, unsigned int *line )
{
	DEBUG_PARSER( "START:%s\n", ( *buf ) );
	SkipToNextToken( buf, line );
	const char *p = PlParseToken( buf, token, size );
	DEBUG_PARSER( "TOKEN:%s\n", p );
	DEBUG_PARSER( "END:%s\n", ( *buf ) );
	return p;
}

static YNNodePropertyType PropertyTypeForString( const char *type )
{
	if ( pl_strcasecmp( type, "string" ) == 0 )
		return YN_NODE_PROP_STR;
	if ( pl_strcasecmp( type, "bool" ) == 0 )
		return YN_NODE_PROP_BOOL;
	if ( pl_strcasecmp( type, "object" ) == 0 )
		return YN_NODE_PROP_OBJ;
	if ( pl_strcasecmp( type, "array" ) == 0 )
		return YN_NODE_PROP_ARRAY;
	if ( pl_strcasecmp( type, "uint8" ) == 0 )
		return YN_NODE_PROP_UI8;
	if ( pl_strcasecmp( type, "uint" ) == 0 || pl_strcasecmp( type, "uint32" ) == 0 )
		return YN_NODE_PROP_UI32;
	if ( pl_strcasecmp( type, "uint64" ) == 0 )
		return YN_NODE_PROP_UI64;
	if ( pl_strcasecmp( type, "int8" ) == 0 )
		return YN_NODE_PROP_I8;
	if ( pl_strcasecmp( type, "int" ) == 0 || pl_strcasecmp( type, "int32" ) == 0 )
		return YN_NODE_PROP_I32;
	if ( pl_strcasecmp( type, "int64" ) == 0 )
		return YN_NODE_PROP_I64;
	if ( pl_strcasecmp( type, "float" ) == 0 )
		return YN_NODE_PROP_F32;
	if ( pl_strcasecmp( type, "float64" ) == 0 )
		return YN_NODE_PROP_F64;

	return YN_NODE_PROP_UNDEFINED;
}

static YNNodeBranch *ParseObjectNode( YNNodeBranch *parent, const char **buf, size_t length, unsigned int currentLine );
static YNNodeBranch *ParseArrayNode( YNNodeBranch *parent, const char **buf, size_t length, unsigned int currentLine )
{
	DEBUG_PARSER( "Entering ParseArrayNode\n" );

	char childType[ NL_MAX_TYPE_LENGTH ];
	if ( ParseToken( buf, childType, sizeof( childType ), &currentLine ) == NULL )
	{
		Warning( "Failed to parse child type for array!\n" );
		return NULL;
	}
	DEBUG_PARSER( "childType( %s )\n", childType );

	char name[ NL_MAX_NAME_LENGTH ];
	if ( ParseToken( buf, name, sizeof( name ), &currentLine ) == NULL )
	{
		Warning( "Failed to parse name!\n" );
		return NULL;
	}
	DEBUG_PARSER( "name( %s )\n", name );

	SkipToNextToken( buf, &currentLine );
	if ( *( *buf ) != '{' )
	{
		Warning( "No opening brace for array, \"%s\"!\n", name );
		return NULL;
	}
	( *buf )++;

	YNNodeBranch *arrayNode = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_ARRAY );
	if ( arrayNode == NULL )
		return NULL;

	SkipToNextToken( buf, &currentLine );

	arrayNode->childType = PropertyTypeForString( childType );
	switch ( arrayNode->childType )
	{
		default:
		{
			Warning( "Invalid child type for array, \"%s\"!\n", name );
			break;
		}
		case YN_NODE_PROP_UI32:
		{
			while ( *( *buf ) != '\0' && *( *buf ) != '}' )
			{
				bool     status;
				uint32_t i = PlParseInteger( buf, &status );
				if ( !status )
				{
					Warning( "Failed to parse integer for array, \"%s\"!\n", name );
					break;
				}
				YnNode_PushBackUI32( arrayNode, NULL, i );
				SkipToNextToken( buf, &currentLine );
			}
			break;
		}
		case YN_NODE_PROP_I32:
		{
			DEBUG_PARSER( "Reading I32\n" );
			while ( *( *buf ) != '\0' && *( *buf ) != '}' )
			{
				bool    status;
				int32_t i = PlParseInteger( buf, &status );
				if ( !status )
				{
					Warning( "Failed to parse integer for array, \"%s\"!\n", name );
					break;
				}
				DEBUG_PARSER( "PushBack Integer: %d\n", i );
				YnNode_PushBackI32( arrayNode, NULL, i );
				SkipToNextToken( buf, &currentLine );
			}
			break;
		}
		case YN_NODE_PROP_F32:
		{
			DEBUG_PARSER( "Reading float\n" );
			while ( *( *buf ) != '\0' && *( *buf ) != '}' )
			{
				bool  status;
				float i = PlParseFloat( buf, &status );
				if ( !status )
				{
					Warning( "Failed to parse integer for array, \"%s\"!\n", name );
					break;
				}
				DEBUG_PARSER( "PushBack Float: %f\n", i );
				YnNode_PushBackF32( arrayNode, NULL, i );
				SkipToNextToken( buf, &currentLine );
			}
			break;
		}
		case YN_NODE_PROP_OBJ:
		{
			DEBUG_PARSER( "Reading object\n" );
			while ( *( *buf ) != '\0' && *( *buf ) != '}' )
			{
				if ( ParseObjectNode( arrayNode, buf, length, 0 ) == NULL )
				{
					Warning( "Failed to parse object node for array, \"%s\"!\n", name );
					break;
				}
				SkipToNextToken( buf, &currentLine );
			}
			break;
		}
		case YN_NODE_PROP_BOOL:
		{
			DEBUG_PARSER( "Reading boolean\n" );
			while ( *( *buf ) != '\0' && *( *buf ) != '}' )
			{
				bool status;
				int  i = PlParseInteger( buf, &status );
				if ( !status )
				{
					Warning( "Failed to parse integer for array, \"%s\"!\n", name );
					break;
				}
				DEBUG_PARSER( "PushBack Boolean: %d\n", i );
				YnNode_PushBackI32( arrayNode, NULL, i );
				SkipToNextToken( buf, &currentLine );
			}
			break;
		}
		case YN_NODE_PROP_STR:
		{
			DEBUG_PARSER( "Reading string\n" );
			do {
				char i[ NL_MAX_STRING_LENGTH ];
				if ( PlParseEnclosedString( buf, i, sizeof( i ) ) == NULL )
				{
					Warning( "Failed to parse enclosed string for array, \"%s\"!\n", name );
					break;
				}
				DEBUG_PARSER( "PushBack String: %s\n", i );
				YnNode_PushBackString( arrayNode, NULL, i );
				SkipToNextToken( buf, &currentLine );
			} while ( *( *buf ) != '\0' && *( *buf ) != '}' );
			break;
		}
		case NL_PROP_LINK:
		{
			DEBUG_PARSER( "Reading link\n" );
			assert( 0 );
			break;
		}
	}

	if ( *( *buf ) != '}' )
	{
		Warning( "No closing brace for array, \"%s\"!\n", name );
		return arrayNode;
	}
	( *buf )++;

	DEBUG_PARSER( "Leaving ParseArrayNode\n" );
	return arrayNode;
}

static YNNodeBranch *ParseNode( YNNodeBranch *parent, const char **buf, size_t length, unsigned int currentLine );
static YNNodeBranch *ParseObjectNode( YNNodeBranch *parent, const char **buf, size_t length, unsigned int currentLine )
{
	DEBUG_PARSER( "Entering ParseObjectNode\n" );

	char name[ NL_MAX_NAME_LENGTH ] = { '\0' };
	if ( parent == NULL || parent->type != YN_NODE_PROP_ARRAY )
	{
		if ( ParseToken( buf, name, sizeof( name ), &currentLine ) == NULL )
		{
			Warning( "Failed to parse name!\n" );
			return NULL;
		}
	}
	DEBUG_PARSER( "name( %s )\n", name );

	/* make sure the object is followed by an opening brace */
	SkipToNextToken( buf, &currentLine );
	if ( *( *buf ) != '{' )
	{
		Warning( "No opening brace for object, \"%s\"!\n", name );
		return NULL;
	}
	( *buf )++;

	YNNodeBranch *objectNode = YnNode_PushBackObject( parent, name );
	if ( objectNode == NULL )
		return NULL;

	/* read in all the children nodes */
	SkipToNextToken( buf, &currentLine );
	while ( *( *buf ) != '\0' && *( *buf ) != '}' )
	{
		if ( ParseNode( objectNode, buf, length, 0 ) == NULL )
		{
			Warning( "Failed to parse child node for object, \"%s\" [%d]!\n", name, currentLine );
			break;
		}

		SkipToNextToken( buf, &currentLine );
	}

	if ( *( *buf ) != '}' )
	{
		Warning( "No closing brace for object, \"%s\"!\n", name );
		return objectNode;
	}
	( *buf )++;

	DEBUG_PARSER( "Leaving ParseObjectNode\n" );
	return objectNode;
}

static YNNodeBranch *ParseNode( YNNodeBranch *parent, const char **buf, size_t length, unsigned int currentLine )
{
	DEBUG_PARSER( "Entering ParseNode\n" );

	/* now try reading in the type */
	char type[ NL_MAX_TYPE_LENGTH ];
	if ( ParseToken( buf, type, sizeof( type ), &currentLine ) == NULL )
		return NULL;
	DEBUG_PARSER( "type( %s )\n", type );

	YNNodePropertyType propertyType = PropertyTypeForString( type );
	/* an array is a special case, parsing-wise */
	if ( propertyType == YN_NODE_PROP_ARRAY )
		return ParseArrayNode( parent, buf, length, currentLine );
	else if ( propertyType == YN_NODE_PROP_OBJ )
		return ParseObjectNode( parent, buf, length, currentLine );
	else
	{
		char name[ NL_MAX_NAME_LENGTH ];
		if ( ParseToken( buf, name, sizeof( name ), &currentLine ) == NULL )
		{
			Warning( "Failed to parse name [%d]!\n", currentLine );
			return NULL;
		}
		DEBUG_PARSER( "name( %s )\n", name );

		/* figure out what data type it is and read in it's result */
		switch ( propertyType )
		{
			case YN_NODE_PROP_UI32:
			{
				bool     status;
				uint32_t i = PlParseInteger( buf, &status );
				if ( !status )
				{
					Warning( "Failed to parse integer, \"%s\" [%d]!\n", name, currentLine );
					return NULL;
				}
				return YnNode_PushBackUI32( parent, name, i );
			}
			case YN_NODE_PROP_I32:
			{
				bool status;
				int  i = PlParseInteger( buf, &status );
				if ( !status )
				{
					Warning( "Failed to parse integer, \"%s\" [%d]!\n", name, currentLine );
					return NULL;
				}
				DEBUG_PARSER( "PushBack Integer: %d\n", i );

				return YnNode_PushBackI32( parent, name, i );
			}
			case YN_NODE_PROP_F32:
			{
				bool  status;
				float i = PlParseFloat( buf, &status );
				if ( !status )
				{
					Warning( "Failed to parse float, \"%s\" [%d]!\n", name, currentLine );
					return NULL;
				}
				DEBUG_PARSER( "PushBack Float: %f\n", i );
				return YnNode_PushBackF32( parent, name, i );
			}
			case YN_NODE_PROP_STR:
			{
				char i[ NL_MAX_STRING_LENGTH ];
				if ( PlParseEnclosedString( buf, i, sizeof( i ) ) == NULL )
				{
					Warning( "Failed to parse string, \"%s\" [%d]!\n", name, currentLine );
					return NULL;
				}
				DEBUG_PARSER( "PushBack String: %s\n", i );
				return YnNode_PushBackString( parent, name, i );
			}
			case YN_NODE_PROP_BOOL:
			{
				char i[ NL_MAX_BOOL_LENGTH ];
				if ( ParseToken( buf, i, sizeof( i ), &currentLine ) == NULL )
				{
					Warning( "Failed to parse boolean, \"%s\" [%d]!\n", name, currentLine );
					return NULL;
				}
				DEBUG_PARSER( "PushBack Boolean: %s\n", i );
				return YnNode_PushBackBool( parent, name, ( pl_strcasecmp( i, "true" ) == 0 || i[ 0 ] == '1' ) );
			}
			default:
				Warning( "Unknown property type, \"%s\" [%d]!\n", type, currentLine );
				break;
		}
	}

	return NULL;
}

YNNodeBranch *YnNode_ParseBuffer( const char *buf, size_t length )
{
	return ParseNode( NULL, &buf, length, 1 );
}
