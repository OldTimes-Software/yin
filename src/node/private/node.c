// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_filesystem.h>

#include "node_private.h"

int nodeLogLevelPrint = -1;
int nodeLogLevelWarn  = -1;
void YnNode_SetupLogs( void )
{
	nodeLogLevelPrint = PlAddLogLevel( "node", PL_COLOUR_DARK_SLATE_BLUE, true );
	nodeLogLevelWarn  = PlAddLogLevel( "node/warning", PL_COLOUR_YELLOW, true );
	Message( "Logs are now active for NODE library\n" );
}

#define YN_NODE_FORMAT_VERSION       1
#define YN_NODE_FORMAT_BINARY_HEADER "node.bin"
#define YN_NODE_FORMAT_ASCII_HEADER  "node.ascii" /* obsolete */
#define YN_NODE_FORMAT_UTF8_HEADER   "node.utf8"

static const char *StringForPropertyType( YNNodePropertyType propertyType )
{
	const char *propToStr[ YN_NODE_MAX_PROPERTY_TYPES ] = {
	        // Special types
	        [YN_NODE_PROP_OBJ]   = "object",
	        [YN_NODE_PROP_STR]   = "string",
	        [YN_NODE_PROP_BOOL]  = "bool",
	        [YN_NODE_PROP_ARRAY] = "array",
	        // Generic types
	        [YN_NODE_PROP_I8]   = "int8",
	        [YN_NODE_PROP_I16]  = "int16",
	        [YN_NODE_PROP_I32]  = "int32",
	        [YN_NODE_PROP_I64]  = "int64",
	        [YN_NODE_PROP_UI8]  = "uint8",
	        [YN_NODE_PROP_UI16] = "uint16",
	        [YN_NODE_PROP_UI32] = "uint32",
	        [YN_NODE_PROP_UI64] = "uint64",
	        [YN_NODE_PROP_F32]  = "float",
	        [YN_NODE_PROP_F64]  = "float64",
	};

	if ( propertyType == YN_NODE_PROP_UNDEFINED )
		return "undefined";

	return propToStr[ propertyType ];
}

static char *nlErrorMsg            = NULL;
static YNNodeErrorCode nlErrorType = YN_NODE_ERROR_SUCCESS;
static void ClearErrorMessage( void )
{
	PlFree( nlErrorMsg );
	nlErrorMsg  = NULL;
	nlErrorType = YN_NODE_ERROR_SUCCESS;
}

static void SetErrorMessage( YNNodeErrorCode type, const char *msg, ... )
{
	ClearErrorMessage();

	nlErrorType = type;

	va_list args;
	va_start( args, msg );

	int length = pl_vscprintf( msg, args ) + 1;
	if ( length <= 0 )
		return;

	nlErrorMsg = PlCAlloc( 1, length, false );
	if ( nlErrorMsg == NULL )
	{
		Warning( "Failed to allocate error message buffer: %d bytes!\n", length );
		return;
	}

	vsnprintf( nlErrorMsg, length, msg, args );
	Warning( "NLERR: %s\n", nlErrorMsg );

	va_end( args );
}

const char *YnNode_GetErrorMessage( void ) { return nlErrorMsg; }
YNNodeErrorCode YnNode_GetError( void ) { return nlErrorType; }

static char *AllocVarString( const char *string, uint16_t *lengthOut )
{
	*lengthOut = ( uint16_t ) strlen( string ) + 1;
	char *buf  = PlCAllocA( 1, *lengthOut );
	strcpy( buf, string );
	return buf;
}

unsigned int YnNode_GetNumOfChildren( const YNNodeBranch *parent )
{
	return PlGetNumLinkedListNodes( parent->linkedList );
}

YNNodeBranch *YnNode_GetFirstChild( YNNodeBranch *parent )
{
	PLLinkedListNode *n = PlGetFirstNode( parent->linkedList );
	if ( n == NULL )
		return NULL;

	return PlGetLinkedListNodeUserData( n );
}

YNNodeBranch *YnNode_GetNextChild( YNNodeBranch *node )
{
	PLLinkedListNode *n = PlGetNextLinkedListNode( node->linkedListNode );
	if ( n == NULL )
		return NULL;

	return PlGetLinkedListNodeUserData( n );
}

YNNodeBranch *YnNode_GetChildByName( YNNodeBranch *parent, const char *name )
{
	if ( parent->type != YN_NODE_PROP_OBJ )
	{
		SetErrorMessage( YN_NODE_ERROR_INVALID_TYPE, "Attempted to get child from an invalid node type!\n" );
		return NULL;
	}

	YNNodeBranch *child = YnNode_GetFirstChild( parent );
	while ( child != NULL )
	{
		if ( strcmp( name, child->name.buf ) == 0 )
			return child;

		child = YnNode_GetNextChild( child );
	}

	return NULL;
}

static const YNNodeVarString *GetValueByName( YNNodeBranch *root, const char *name )
{
	const YNNodeBranch *field = YnNode_GetChildByName( root, name );
	if ( field == NULL )
		return NULL;

	return &field->data;
}

YNNodeBranch *YnNode_GetParent( YNNodeBranch *node )
{
	return node->parent;
}

const char *YnNode_GetName( const YNNodeBranch *node )
{
	return node->name.buf;
}

YNNodePropertyType YnNode_GetType( const YNNodeBranch *node )
{
	return node->type;
}

YNNodeErrorCode YnNode_GetStr( const YNNodeBranch *node, char *dest, size_t length )
{
	if ( node->type != YN_NODE_PROP_STR ) return YN_NODE_ERROR_INVALID_TYPE;
	snprintf( dest, length, "%s", node->data.buf );
	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetBool( const YNNodeBranch *node, bool *dest )
{
	if ( node->type != YN_NODE_PROP_BOOL ) return YN_NODE_ERROR_INVALID_TYPE;

	if ( ( strcmp( node->data.buf, "true" ) == 0 ) || ( node->data.buf[ 0 ] == '1' && node->data.buf[ 1 ] == '\0' ) )
	{
		*dest = true;
		return YN_NODE_ERROR_SUCCESS;
	}
	else if ( ( strcmp( node->data.buf, "false" ) == 0 ) || ( node->data.buf[ 0 ] == '0' && node->data.buf[ 1 ] == '\0' ) )
	{
		*dest = false;
		return YN_NODE_ERROR_SUCCESS;
	}

	SetErrorMessage( YN_NODE_ERROR_INVALID_ARGUMENT, "Invalid data passed from var" );
	return YN_NODE_ERROR_INVALID_ARGUMENT;
}

YNNodeErrorCode YnNode_GetF32( const YNNodeBranch *node, float *dest )
{
	if ( node->type != YN_NODE_PROP_F32 ) return YN_NODE_ERROR_INVALID_TYPE;
	*dest = strtof( node->data.buf, NULL );
	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetF64( const YNNodeBranch *node, double *dest )
{
	if ( node->type != YN_NODE_PROP_F64 ) return YN_NODE_ERROR_INVALID_TYPE;
	*dest = strtod( node->data.buf, NULL );
	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetI8( const YNNodeBranch *node, int8_t *dest )
{
	if ( node->type != YN_NODE_PROP_I8 ) return YN_NODE_ERROR_INVALID_TYPE;
	*dest = ( int8_t ) strtol( node->data.buf, NULL, 10 );
	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetI16( const YNNodeBranch *node, int16_t *dest )
{
	if ( node->type != YN_NODE_PROP_I16 ) return YN_NODE_ERROR_INVALID_TYPE;
	*dest = ( int16_t ) strtol( node->data.buf, NULL, 10 );
	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetI32( const YNNodeBranch *node, int32_t *dest )
{
	if ( node->type != YN_NODE_PROP_I32 ) return YN_NODE_ERROR_INVALID_TYPE;
	*dest = ( int32_t ) strtol( node->data.buf, NULL, 10 );
	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetI64( const YNNodeBranch *node, int64_t *dest )
{
	if ( node->type != YN_NODE_PROP_I64 ) return YN_NODE_ERROR_INVALID_TYPE;
	*dest = strtoll( node->data.buf, NULL, 10 );
	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetUI8( const YNNodeBranch *node, uint8_t *dest )
{
	if ( node->type != YN_NODE_PROP_UI8 ) return YN_NODE_ERROR_INVALID_TYPE;
	*dest = ( uint8_t ) strtoul( node->data.buf, NULL, 10 );
	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetUI16( const YNNodeBranch *node, uint16_t *dest )
{
	if ( node->type != YN_NODE_PROP_UI16 ) return YN_NODE_ERROR_INVALID_TYPE;
	*dest = ( uint16_t ) strtoul( node->data.buf, NULL, 10 );
	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetUI32( const YNNodeBranch *node, uint32_t *dest )
{
	if ( node->type != YN_NODE_PROP_UI32 ) return YN_NODE_ERROR_INVALID_TYPE;
	*dest = strtoul( node->data.buf, NULL, 10 );
	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetUI64( const YNNodeBranch *node, uint64_t *dest )
{
	if ( node->type != YN_NODE_PROP_UI64 ) return YN_NODE_ERROR_INVALID_TYPE;
	*dest = strtoull( node->data.buf, NULL, 10 );
	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetStrArray( YNNodeBranch *parent, const char **buf, unsigned int numElements )
{
	if ( parent->type != YN_NODE_PROP_ARRAY || parent->childType != YN_NODE_PROP_STR )
		return YN_NODE_ERROR_INVALID_TYPE;

	YNNodeBranch *child = YnNode_GetFirstChild( parent );
	for ( unsigned int i = 0; i < numElements; ++i )
	{
		if ( child == NULL )
			return YN_NODE_ERROR_INVALID_ELEMENTS;

		buf[ i ] = child->data.buf;

		child = YnNode_GetNextChild( child );
	}

	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetI8Array( YNNodeBranch *parent, int8_t *buf, unsigned int numElements )
{
	if ( parent->type != YN_NODE_PROP_ARRAY || parent->childType != YN_NODE_PROP_I8 )
		return YN_NODE_ERROR_INVALID_TYPE;

	YNNodeBranch *child = YnNode_GetFirstChild( parent );
	for ( unsigned int i = 0; i < numElements; ++i )
	{
		if ( child == NULL )
			return YN_NODE_ERROR_INVALID_ELEMENTS;

		YNNodeErrorCode errorCode = YnNode_GetI8( child, &buf[ i ] );
		if ( errorCode != YN_NODE_ERROR_SUCCESS )
			return errorCode;

		child = YnNode_GetNextChild( child );
	}

	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetI16Array( YNNodeBranch *parent, int16_t *buf, unsigned int numElements )
{
	if ( parent->type != YN_NODE_PROP_ARRAY || parent->childType != YN_NODE_PROP_I16 )
		return YN_NODE_ERROR_INVALID_TYPE;

	YNNodeBranch *child = YnNode_GetFirstChild( parent );
	for ( unsigned int i = 0; i < numElements; ++i )
	{
		if ( child == NULL )
			return YN_NODE_ERROR_INVALID_ELEMENTS;

		YNNodeErrorCode errorCode = YnNode_GetI16( child, &buf[ i ] );
		if ( errorCode != YN_NODE_ERROR_SUCCESS )
			return errorCode;

		child = YnNode_GetNextChild( child );
	}

	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetI32Array( YNNodeBranch *parent, int32_t *buf, unsigned int numElements )
{
	if ( parent->type != YN_NODE_PROP_ARRAY || parent->childType != YN_NODE_PROP_I32 )
		return YN_NODE_ERROR_INVALID_TYPE;

	YNNodeBranch *child = YnNode_GetFirstChild( parent );
	for ( unsigned int i = 0; i < numElements; ++i )
	{
		if ( child == NULL )
			return YN_NODE_ERROR_INVALID_ELEMENTS;

		YNNodeErrorCode errorCode = YnNode_GetI32( child, &buf[ i ] );
		if ( errorCode != YN_NODE_ERROR_SUCCESS )
			return errorCode;

		child = YnNode_GetNextChild( child );
	}

	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetUI32Array( YNNodeBranch *parent, uint32_t *buf, unsigned int numElements )
{
	if ( parent->type != YN_NODE_PROP_ARRAY || parent->childType != YN_NODE_PROP_UI32 )
		return YN_NODE_ERROR_INVALID_TYPE;

	YNNodeBranch *child = YnNode_GetFirstChild( parent );
	for ( unsigned int i = 0; i < numElements; ++i )
	{
		if ( child == NULL )
			return YN_NODE_ERROR_INVALID_ELEMENTS;

		YNNodeErrorCode errorCode = YnNode_GetUI32( child, &buf[ i ] );
		if ( errorCode != YN_NODE_ERROR_SUCCESS )
			return errorCode;

		child = YnNode_GetNextChild( child );
	}

	return YN_NODE_ERROR_SUCCESS;
}

YNNodeErrorCode YnNode_GetF32Array( YNNodeBranch *parent, float *buf, unsigned int numElements )
{
	if ( parent->type != YN_NODE_PROP_ARRAY || parent->childType != YN_NODE_PROP_F32 )
		return YN_NODE_ERROR_INVALID_TYPE;

	YNNodeBranch *child = YnNode_GetFirstChild( parent );
	for ( unsigned int i = 0; i < numElements; ++i )
	{
		if ( child == NULL )
			return YN_NODE_ERROR_INVALID_ELEMENTS;

		YNNodeErrorCode errorCode = YnNode_GetF32( child, &buf[ i ] );
		if ( errorCode != YN_NODE_ERROR_SUCCESS )
			return errorCode;

		child = YnNode_GetNextChild( child );
	}

	return YN_NODE_ERROR_SUCCESS;
}

/******************************************/
/** Get: ByName **/

bool YnNode_GetBoolByName( YNNodeBranch *root, const char *name, bool fallback )
{
	const YNNodeBranch *child = YnNode_GetChildByName( root, name );
	if ( child == NULL )
		return fallback;

	bool out;
	if ( YnNode_GetBool( child, &out ) != YN_NODE_ERROR_SUCCESS )
		return fallback;

	return out;
}

const char *YnNode_GetStringByName( YNNodeBranch *node, const char *name, const char *fallback )
{
	/* todo: warning on fail */
	const YNNodeVarString *var = GetValueByName( node, name );
	return ( var != NULL ) ? var->buf : fallback;
}

float YnNode_GetF32ByName( YNNodeBranch *node, const char *name, float fallback )
{
	/* todo: warning on fail */
	const YNNodeVarString *var = GetValueByName( node, name );
	return ( var != NULL ) ? strtof( var->buf, NULL ) : fallback;
}

int32_t YnNode_GetI32ByName( YNNodeBranch *node, const char *name, int32_t fallback )
{
	/* todo: warning on fail */
	const YNNodeVarString *var = GetValueByName( node, name );
	return ( var != NULL ) ? strtol( var->buf, NULL, 10 ) : fallback;
}

/******************************************/

YNNodeBranch *YnNode_PushBackNewBranch( YNNodeBranch *parent, const char *name, YNNodePropertyType propertyType )
{
	/* arrays are special cases */
	if ( parent != NULL && parent->type == YN_NODE_PROP_ARRAY && propertyType != parent->childType )
	{
		SetErrorMessage( YN_NODE_ERROR_INVALID_TYPE, "attempted to add invalid type (%s)", StringForPropertyType( propertyType ) );
		return NULL;
	}

	YNNodeBranch *node = PlCAllocA( 1, sizeof( YNNodeBranch ) );

	/* assign the node name, if provided */
	if ( ( parent == NULL || parent->type != YN_NODE_PROP_ARRAY ) && name != NULL )
		node->name.buf = AllocVarString( name, &node->name.length );

	node->type       = propertyType;
	node->linkedList = PlCreateLinkedList();

	/* if root is provided, this is treated as a child of that node */
	if ( parent != NULL )
	{
		if ( parent->linkedList == NULL )
			parent->linkedList = PlCreateLinkedList();

		node->linkedListNode = PlInsertLinkedListNode( parent->linkedList, node );
		node->parent         = parent;
	}

	return node;
}

YNNodeBranch *YnNode_PushBackBranch( YNNodeBranch *parent, YNNodeBranch *child )
{
	YNNodeBranch *childCopy   = YnNode_CopyBranch( child );
	childCopy->parent         = parent;
	childCopy->linkedListNode = PlInsertLinkedListNode( parent->linkedList, childCopy );
	return childCopy;
}

YNNodeBranch *YnNode_PushBackObject( YNNodeBranch *node, const char *name )
{
	return YnNode_PushBackNewBranch( node, name, YN_NODE_PROP_OBJ );
}

YNNodeBranch *YnNode_PushBackString( YNNodeBranch *parent, const char *name, const char *var )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_STR );
	if ( node != NULL )
		node->data.buf = AllocVarString( var, &node->data.length );

	return node;
}

YNNodeBranch *YnNode_PushBackStringArray( YNNodeBranch *parent, const char *name, const char **array, unsigned int numElements )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_ARRAY );
	if ( node != NULL )
	{
		node->childType = YN_NODE_PROP_STR;
		for ( unsigned int i = 0; i < numElements; ++i )
			YnNode_PushBackString( node, NULL, array[ i ] );
	}
	return node;
}

YNNodeBranch *YnNode_PushBackBool( YNNodeBranch *parent, const char *name, bool var )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_BOOL );
	if ( node != NULL )
		node->data.buf = AllocVarString( var ? "true" : "false", &node->data.length );

	return node;
}

YNNodeBranch *YnNode_PushBackI8( YNNodeBranch *parent, const char *name, int8_t var )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_I8 );
	if ( node != NULL )
	{
		char buf[ 4 ];
		pl_itoa( var, buf, sizeof( buf ), 10 );
		node->data.buf = AllocVarString( buf, &node->data.length );
	}
	return node;
}

YNNodeBranch *YnNode_PushBackI16( YNNodeBranch *parent, const char *name, int16_t var )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_I16 );
	if ( node != NULL )
	{
		char buf[ 32 ];
		snprintf( buf, sizeof( buf ), PL_FMT_int16, var );
		node->data.buf = AllocVarString( buf, &node->data.length );
	}
	return node;
}

YNNodeBranch *YnNode_PushBackI32( YNNodeBranch *parent, const char *name, int32_t var )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_I32 );
	if ( node != NULL )
	{
		char buf[ 32 ];
		snprintf( buf, sizeof( buf ), PL_FMT_int32, var );
		node->data.buf = AllocVarString( buf, &node->data.length );
	}
	return node;
}

YNNodeBranch *YnNode_PushBackUI32( YNNodeBranch *parent, const char *name, uint32_t var )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_UI32 );
	if ( node != NULL )
	{
		char buf[ 32 ];
		snprintf( buf, sizeof( buf ), PL_FMT_uint32, var );
		node->data.buf = AllocVarString( buf, &node->data.length );
	}
	return node;
}

YNNodeBranch *YnNode_PushBackF32( YNNodeBranch *parent, const char *name, float var )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_F32 );
	if ( node != NULL )
	{
		char buf[ 32 ];
		snprintf( buf, sizeof( buf ), PL_FMT_float, var );
		node->data.buf = AllocVarString( buf, &node->data.length );
	}
	return node;
}

YNNodeBranch *YnNode_PushBackF64( YNNodeBranch *parent, const char *name, double var )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_F64 );
	if ( node != NULL )
	{
		char buf[ 32 ];
		snprintf( buf, sizeof( buf ), PL_FMT_double, var );
		node->data.buf = AllocVarString( buf, &node->data.length );
	}
	return node;
}

YNNodeBranch *YnNode_PushBackI32Array( YNNodeBranch *parent, const char *name, const int *array, unsigned int numElements )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_ARRAY );
	if ( node != NULL )
	{
		node->childType = YN_NODE_PROP_I32;
		for ( unsigned int i = 0; i < numElements; ++i )
			YnNode_PushBackI32( node, NULL, array[ i ] );
	}
	return node;
}

YNNodeBranch *YnNode_PushBackF32Array( YNNodeBranch *parent, const char *name, const float *array, unsigned int numElements )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_ARRAY );
	if ( node != NULL )
	{
		node->childType = YN_NODE_PROP_F32;
		for ( unsigned int i = 0; i < numElements; ++i )
			YnNode_PushBackF32( node, NULL, array[ i ] );
	}
	return node;
}

YNNodeBranch *YnNode_PushBackObjectArray( YNNodeBranch *parent, const char *name )
{
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, name, YN_NODE_PROP_ARRAY );
	if ( node != NULL )
		node->childType = YN_NODE_PROP_OBJ;

	return node;
}

static char *CopyVarString( const YNNodeVarString *varString, uint16_t *length )
{
	*length   = varString->length;
	char *buf = PL_NEW_( char, *length + 1 );
	strncpy( buf, varString->buf, *length );
	return buf;
}

/**
 * Copies the given node list.
 */
YNNodeBranch *YnNode_CopyBranch( YNNodeBranch *node )
{
	YNNodeBranch *newNode = PL_NEW( YNNodeBranch );
	newNode->type         = node->type;
	newNode->childType    = node->childType;
	newNode->data.buf     = CopyVarString( &node->data, &newNode->data.length );
	newNode->name.buf     = CopyVarString( &node->name, &newNode->name.length );
	// Not setting the parent is intentional here, since we likely don't want that link

	YNNodeBranch *child = YnNode_GetFirstChild( node );
	while ( child != NULL )
	{
		if ( newNode->linkedList == NULL )
			newNode->linkedList = PlCreateLinkedList();

		YNNodeBranch *newChild   = YnNode_CopyBranch( child );
		newChild->linkedListNode = PlInsertLinkedListNode( newNode->linkedList, newChild );
		newChild->parent         = newNode;

		child = YnNode_GetNextChild( child );
	}

	return newNode;
}

void YnNode_DestroyBranch( YNNodeBranch *node )
{
	PlFree( node->name.buf );
	PlFree( node->data.buf );

	/* if it's an object/array, we'll need to clean up all it's children */
	if ( node->type == YN_NODE_PROP_OBJ || node->type == YN_NODE_PROP_ARRAY )
	{
		YNNodeBranch *child = YnNode_GetFirstChild( node );
		while ( child != NULL )
		{
			YNNodeBranch *nextChild = YnNode_GetNextChild( child );
			YnNode_DestroyBranch( child );
			child = nextChild;
		}
	}

	PlDestroyLinkedList( node->linkedList );
	if ( node->parent != NULL )
		PlDestroyLinkedListNode( node->linkedListNode );

	PlFree( node );
}

/******************************************/
/** Deserialisation **/

static char *DeserializeStringVar( PLFile *file, uint16_t *length )
{
	*length = PlReadInt16( file, false, NULL );
	if ( *length > 0 )
	{
		char *buf = PlMAlloc( *length, true );
		PlReadFile( file, buf, sizeof( char ), *length );
		return buf;
	}

	return NULL;
}

static YNNodeBranch *DeserializeBinaryNode( PLFile *file, YNNodeBranch *parent )
{
	/* try to fetch the name, not all nodes necessarily have a name... */
	YNNodeVarString name;
	name.buf          = DeserializeStringVar( file, &name.length );
	const char *dname = ( name.buf != NULL ) ? name.buf : "unknown";

	bool status;
	YNNodePropertyType type = ( YNNodePropertyType ) PlReadInt8( file, &status );
	if ( !status )
	{
		Warning( "Failed to read property type for \"%s\"!\n", dname );
		PlFree( name.buf );
		return NULL;
	}

	/* binary implementation is pretty damn straight forward */
	YNNodeBranch *node = YnNode_PushBackNewBranch( parent, NULL, type );
	if ( node == NULL )
	{
		PlFree( name.buf );
		return NULL;
	}

	/* node now takes ownership of name */
	node->name = name;

	switch ( node->type )
	{
		default:
			Warning( "Encountered unhandled node type: %d!\n", node->type );
			YnNode_DestroyBranch( node );
			node = NULL;
			break;
		case YN_NODE_PROP_ARRAY:
			/* only extra component we get here is the child type */
			node->childType = ( YNNodePropertyType ) PlReadInt8( file, NULL );
		case YN_NODE_PROP_OBJ:
		{
			unsigned int numChildren = PlReadInt32( file, false, NULL );
			for ( unsigned int i = 0; i < numChildren; ++i )
				DeserializeBinaryNode( file, node );
			break;
		}
		case YN_NODE_PROP_STR:
		{
			node->data.buf = DeserializeStringVar( file, &node->data.length );
			break;
		}
		case YN_NODE_PROP_BOOL:
		{
			bool v         = PlReadInt8( file, NULL );
			node->data.buf = AllocVarString( v ? "true" : "false", &node->data.length );
			break;
		}
		case YN_NODE_PROP_F32:
		{
			float v = PlReadFloat32( file, false, NULL );
			char str[ 32 ];
			snprintf( str, sizeof( str ), PL_FMT_float, v );
			node->data.buf = AllocVarString( str, &node->data.length );
			break;
		}
		case YN_NODE_PROP_F64:
		{
			double v = PlReadFloat64( file, false, NULL );
			char str[ 32 ];
			snprintf( str, sizeof( str ), PL_FMT_double, v );
			node->data.buf = AllocVarString( str, &node->data.length );
			break;
		}
		case YN_NODE_PROP_I8:
		{
			int8_t v = PlReadInt8( file, NULL );
			char str[ 32 ];
			snprintf( str, sizeof( str ), PL_FMT_int32, v );
			node->data.buf = AllocVarString( str, &node->data.length );
			break;
		}
		case YN_NODE_PROP_I32:
		{
			int32_t v = PlReadInt32( file, false, NULL );
			char str[ 32 ];
			snprintf( str, sizeof( str ), PL_FMT_int32, v );
			node->data.buf = AllocVarString( str, &node->data.length );
			break;
		}
		case YN_NODE_PROP_I64:
		{
			int64_t v = PlReadInt64( file, false, NULL );
			char str[ 32 ];
			snprintf( str, sizeof( str ), PL_FMT_int64, v );
			node->data.buf = AllocVarString( str, &node->data.length );
			break;
		}
	}

	return node;
}

static NLFileType ParseNodeFileType( PLFile *file )
{
	char token[ 32 ];
	if ( PlReadString( file, token, sizeof( token ) ) == NULL )
	{
		SetErrorMessage( YN_NODE_ERROR_IO_READ, "Failed to read in file type: %s", PlGetError() );
		return YN_NODE_FILE_INVALID;
	}

	if ( strncmp( token, YN_NODE_FORMAT_BINARY_HEADER, strlen( YN_NODE_FORMAT_BINARY_HEADER ) ) == 0 )
		return YN_NODE_FILE_BINARY;
	/* we still check for 'ascii' here, just for backwards compat, but they're handled the
	 * same either way */
	else if ( strncmp( token, YN_NODE_FORMAT_ASCII_HEADER, strlen( YN_NODE_FORMAT_ASCII_HEADER ) ) == 0 ||
	          strncmp( token, YN_NODE_FORMAT_UTF8_HEADER, strlen( YN_NODE_FORMAT_UTF8_HEADER ) ) == 0 )
		return YN_NODE_FILE_UTF8;

	SetErrorMessage( YN_NODE_ERROR_INVALID_ARGUMENT, "Unknown file type \"%s\"", token );
	return YN_NODE_FILE_INVALID;
}

YNNodeBranch *YnNode_ParseFile( PLFile *file, const char *objectType )
{
	YNNodeBranch *root = NULL;

	NLFileType fileType = ParseNodeFileType( file );
	if ( fileType == YN_NODE_FILE_BINARY )
		root = DeserializeBinaryNode( file, NULL );
	else if ( fileType == YN_NODE_FILE_UTF8 )
	{
		/* first need to run the pre-processor on it */
		size_t length = PlGetFileSize( file );
		if ( length <= strlen( YN_NODE_FORMAT_ASCII_HEADER ) )
			Warning( "Unexpected file size, possibly not a valid node file?\n" );
		else
		{
			const char *data = ( const char * ) ( ( uint8_t * ) PlGetFileData( file ) + strlen( YN_NODE_FORMAT_ASCII_HEADER ) );
			char *buf        = PL_NEW_( char, length + 1 );
			memcpy( buf, data, length );
			buf  = YnNode_PreProcessScript( buf, &length, true );
			root = YnNode_ParseBuffer( buf, length );
			PL_DELETE( buf );
		}
	}
	else
		Warning( "Invalid node file type: %d\n", fileType );

	if ( root != NULL && objectType != NULL )
	{
		const char *rootName = YnNode_GetName( root );
		if ( strcmp( rootName, objectType ) != 0 )
		{
			/* destroy the tree */
			YnNode_DestroyBranch( root );

			Warning( "Invalid \"%s\" file, expected \"%s\" but got \"%s\"!\n", objectType, objectType, rootName );
			return NULL;
		}
	}

	return root;
}

YNNodeBranch *YnNode_LoadFile( const char *path, const char *objectType )
{
	ClearErrorMessage();

	PLFile *file = PlOpenFile( path, true );
	if ( file == NULL )
	{
		Warning( "Failed to open \"%s\": %s\n", path, PlGetError() );
		return NULL;
	}

	YNNodeBranch *root = YnNode_ParseFile( file, objectType );

	PlCloseFile( file );

	return root;
}

/******************************************/
/** Serialisation **/

static unsigned int sDepth; /* serialisation depth */

static void WriteLine( FILE *file, const char *string, bool tabify )
{
	if ( tabify )
	{
		for ( unsigned int i = 0; i < sDepth; ++i )
			fputc( '\t', file );
	}

	if ( string == NULL )
		return;

	fprintf( file, "%s", string );
}

static void SerializeStringVar( const YNNodeVarString *string, NLFileType fileType, FILE *file )
{
	if ( fileType == YN_NODE_FILE_BINARY )
	{
		fwrite( &string->length, sizeof( uint16_t ), 1, file );
		/* slightly paranoid here, because strBuf is probably null if length is 0
		 * which is totally valid, but eh */
		if ( string->length > 0 )
			fwrite( string->buf, sizeof( char ), string->length, file );

		return;
	}

	/* allow nameless nodes, used for arrays */
	if ( string->length == 0 )
		return;

	bool encloseString = false;
	const char *c      = string->buf;
	if ( *c == '\0' )
		/* enclose an empty string!!! */
		encloseString = true;
	else
	{
		/* otherwise, check if there are any spaces */
		while ( *c != '\0' )
		{
			if ( *c == ' ' )
			{
				encloseString = true;
				break;
			}

			c++;
		}
	}

	if ( encloseString )
		fprintf( file, "\"%s\" ", string->buf );
	else
		fprintf( file, "%s ", string->buf );
}

static void SerializeNodeTree( FILE *file, YNNodeBranch *root, NLFileType fileType );
static void SerializeNode( FILE *file, YNNodeBranch *node, NLFileType fileType )
{
	if ( fileType == YN_NODE_FILE_UTF8 )
	{
		/* write out the line identifying this node */
		WriteLine( file, NULL, true );
		YNNodeBranch *parent = YnNode_GetParent( node );
		if ( parent == NULL || parent->type != YN_NODE_PROP_ARRAY )
		{
			fprintf( file, "%s ", StringForPropertyType( node->type ) );
			if ( node->type == YN_NODE_PROP_ARRAY )
				fprintf( file, "%s ", StringForPropertyType( node->childType ) );

			SerializeStringVar( &node->name, fileType, file );
		}

		/* if this node has children, serialize all those */
		if ( node->type == YN_NODE_PROP_OBJ || node->type == YN_NODE_PROP_ARRAY )
		{
			WriteLine( file, "{\n", ( parent != NULL && parent->type == YN_NODE_PROP_ARRAY ) );
			sDepth++;
			SerializeNodeTree( file, node, fileType );
			sDepth--;
			WriteLine( file, "}\n", true );
		}
		else
		{
			SerializeStringVar( &node->data, fileType, file );
			fprintf( file, "\n" );
		}

		return;
	}

	SerializeStringVar( &node->name, fileType, file );
	fwrite( &node->type, sizeof( int8_t ), 1, file );
	switch ( node->type )
	{
		default:
			Warning( "Invalid node type: " PL_FMT_uint32 "/n", node->type );
			abort();
		case YN_NODE_PROP_F32:
		{
			float v;
			YnNode_GetF32( node, &v );
			fwrite( &v, sizeof( float ), 1, file );
			break;
		}
		case YN_NODE_PROP_F64:
		{
			double v;
			YnNode_GetF64( node, &v );
			fwrite( &v, sizeof( double ), 1, file );
			break;
		}
		case YN_NODE_PROP_I8:
		{
			int8_t v;
			YnNode_GetI8( node, &v );
			fwrite( &v, sizeof( int8_t ), 1, file );
			break;
		}
		case YN_NODE_PROP_I16:
		{
			int16_t v;
			YnNode_GetI16( node, &v );
			fwrite( &v, sizeof( int16_t ), 1, file );
		}
		case YN_NODE_PROP_I32:
		{
			int32_t v;
			YnNode_GetI32( node, &v );
			fwrite( &v, sizeof( int32_t ), 1, file );
			break;
		}
		case YN_NODE_PROP_I64:
		{
			int64_t v;
			YnNode_GetI64( node, &v );
			fwrite( &v, sizeof( int64_t ), 1, file );
			break;
		}
		case YN_NODE_PROP_UI8:
		{
			uint8_t v;
			YnNode_GetUI8( node, &v );
			fwrite( &v, sizeof( uint8_t ), 1, file );
			break;
		}
		case YN_NODE_PROP_UI16:
		{
			uint16_t v;
			YnNode_GetUI16( node, &v );
			fwrite( &v, sizeof( uint16_t ), 1, file );
		}
		case YN_NODE_PROP_UI32:
		{
			uint32_t v;
			YnNode_GetUI32( node, &v );
			fwrite( &v, sizeof( uint32_t ), 1, file );
			break;
		}
		case YN_NODE_PROP_UI64:
		{
			uint64_t v;
			YnNode_GetUI64( node, &v );
			fwrite( &v, sizeof( uint64_t ), 1, file );
			break;
		}
		case YN_NODE_PROP_STR:
		{
			SerializeStringVar( &node->data, fileType, file );
			break;
		}
		case YN_NODE_PROP_BOOL:
		{
			bool v;
			YnNode_GetBool( node, &v );
			fwrite( &v, sizeof( uint8_t ), 1, file );
			break;
		}
		case YN_NODE_PROP_ARRAY:
			/* only extra component here is the child type */
			fwrite( &node->childType, sizeof( uint8_t ), 1, file );
		case YN_NODE_PROP_OBJ:
		{
			uint32_t i = PlGetNumLinkedListNodes( node->linkedList );
			fwrite( &i, sizeof( uint32_t ), 1, file );
			SerializeNodeTree( file, node, fileType );
			break;
		}
	}
}

static void SerializeNodeTree( FILE *file, YNNodeBranch *root, NLFileType fileType )
{
	PLLinkedListNode *i = PlGetFirstNode( root->linkedList );
	while ( i != NULL )
	{
		YNNodeBranch *node = PlGetLinkedListNodeUserData( i );
		SerializeNode( file, node, fileType );
		i = PlGetNextLinkedListNode( i );
	}
}

/**
 * Serialize the given node set.
 */
bool YnNode_WriteFile( const char *path, YNNodeBranch *root, NLFileType fileType )
{
	FILE *file = fopen( path, "wb" );
	if ( file == NULL )
	{
		SetErrorMessage( YN_NODE_ERROR_IO_WRITE, "Failed to open path \"%s\"", path );
		return false;
	}

	if ( fileType == YN_NODE_FILE_BINARY )
		fprintf( file, YN_NODE_FORMAT_BINARY_HEADER "\n" );
	else
	{
		sDepth = 0;
		fprintf( file, YN_NODE_FORMAT_UTF8_HEADER "\n; this node file has been auto-generated!\n" );
	}

	SerializeNode( file, root, fileType );

	fclose( file );

	return true;
}

/******************************************/
/** API Testing **/

void YnNode_PrintTree( YNNodeBranch *node, int index )
{
	for ( int i = 0; i < index; ++i ) printf( "\t" );
	if ( node->type == YN_NODE_PROP_OBJ || node->type == YN_NODE_PROP_ARRAY )
	{
		index++;

		const char *name = ( node->name.buf != NULL ) ? node->name.buf : "";
		if ( node->type == YN_NODE_PROP_OBJ )
			Message( "%s (%s)\n", name, StringForPropertyType( node->type ) );
		else
			Message( "%s (%s %s)\n", name, StringForPropertyType( node->type ), StringForPropertyType( node->childType ) );

		YNNodeBranch *child = YnNode_GetFirstChild( node );
		while ( child != NULL )
		{
			YnNode_PrintTree( child, index );
			child = YnNode_GetNextChild( child );
		}
	}
	else
	{
		YNNodeBranch *parent = YnNode_GetParent( node );
		if ( parent != NULL && parent->type == YN_NODE_PROP_ARRAY )
			Message( "%s %s\n", StringForPropertyType( node->type ), node->data.buf );
		else
			Message( "%s %s %s\n", StringForPropertyType( node->type ), node->name.buf, node->data.buf );
	}
}
