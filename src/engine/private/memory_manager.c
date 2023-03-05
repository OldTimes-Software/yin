/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plcore/pl_linkedlist.h>

#include "engine_private.h"

static PLMemoryGroup *memoryGroups[ MEM_CACHE_END ];

/* ======================================================================
 * Cache Pools
 * ====================================================================*/

static PLLinkedList *memCachePools[ MEM_CACHE_END ];

static void InitializeCachePools( void )
{
	for ( unsigned int i = 0; i < MEM_CACHE_END; ++i )
	{
		memCachePools[ i ] = PlCreateLinkedList();
		if ( memCachePools[ i ] == NULL )
			PRINT_ERROR( "Failed to create cache pool: " PL_FMT_int32 "\nPL: %s\n", i, PlGetError() );
	}
}

void MM_AddToCache( const char *id, uint8_t pool, void *data )
{
	/* ensure the data hasn't been cached already */
	void *cachedData = MM_GetCachedData( id, pool );
	if ( cachedData != NULL )
		PRINT_ERROR( "Attempted to cache duplicate data: %s\n", id );

	MMCacheHeader *header = PL_NEW( MMCacheHeader );
	header->id            = PlGenerateHashSDBM( id );
	header->pool          = pool;
	header->userData      = data;
	snprintf( header->description, sizeof( header->description ), "%s", id );

	PLLinkedListNode *node = PlInsertLinkedListNode( memCachePools[ pool ], header );
	if ( node == NULL )
		PRINT_ERROR( "Failed to insert node for cache pool!\n" );

	PRINT( "Added \"%s\" (%u) to cache pool %u\n", id, header->id, pool );
}

static MMCacheHeader *GetCache( uint32_t id, uint8_t pool )
{
	PLLinkedListNode *node = PlGetFirstNode( memCachePools[ pool ] );
	while ( node != NULL )
	{
		MMCacheHeader *header = PlGetLinkedListNodeUserData( node );
		if ( header->id == id )
			return header;

		node = PlGetNextLinkedListNode( node );
	}

	return NULL;
}

void *MM_GetCachedData( const char *id, uint8_t pool )
{
	uint32_t       hashedName = PlGenerateHashSDBM( id );
	MMCacheHeader *header     = GetCache( hashedName, pool );
	if ( header != NULL )
		return header->userData;

	return NULL;
}

static void RemoveFromCache( uint32_t id, uint8_t pool )
{
	MMCacheHeader *header = GetCache( id, pool );
	if ( header == NULL )
	{
		PRINT_WARNING( "Attempted to remove node from cache pool, but failed: %s\n", id );
		return;
	}

	PlDestroyLinkedListNode( header->node );

	PRINT( "Removed \"%s\" from cache\n", header->description );

	PL_DELETE( header );
}

/* ======================================================================
 * Reference Counting and Garbage Collection
 * ====================================================================*/

static PLLinkedList *mmReferenceList;

#define MEM_CLEANUP_DELAY 200.0

//#define DEBUG_MEMORY

static bool FreeReference( MMReference *m, bool force )
{
#if defined( DEBUG_MEMORY )
	DebugMsg( "%s, numRefs = %d, ttl = %u\n",
	          m->description[ 0 ] == '\0' ? "unknown" : m->description,
	          m->numRefs,
	          m->ttl );
#endif

	if ( m->numReferences <= 0 && ( force || m->timeToLive < Engine_GetNumTicks() ) )
	{
		/* remove it from whatever cached list it exists in */
		if ( m->cache != NULL )
		{
			RemoveFromCache( m->cache->id, m->cache->pool );
			m->cache = NULL;
		}

		PLLinkedListNode *node = m->node;
		m->cleanupFunction( m->userData );
		PlDestroyLinkedListNode( node );

		return true;
	}

	return false;
}

static void CleanupUnreferencedResources( bool force )
{
	PLLinkedListNode *child = PlGetFirstNode( mmReferenceList );
	while ( child != NULL )
	{
		PLLinkedListNode *nextChild = PlGetNextLinkedListNode( child );
		MMReference      *m         = PlGetLinkedListNodeUserData( child );

#if defined( DEBUG_MEMORY )
		DebugMsg( " %s (" COM_FMT_int32 ")\n", m->description, m->numRefs );
#endif

		FreeReference( m, force );

		child = nextChild;
	}
}

#define MEM_CLEANUP_TASK_NAME "mem_cleanup"

static void MEM_CB_Cleanup( void *unused0, double unused1 )
{
	( void )( unused0 );
	( void )( unused1 );

	CleanupUnreferencedResources( false );

	Sch_PushTask( MEM_CLEANUP_TASK_NAME, MEM_CB_Cleanup, NULL, MEM_CLEANUP_DELAY );
}

void MemoryManager_Initialize( void )
{
	PRINT( "Initializing memory manager\n" );

	for ( unsigned int i = 0; i < MEM_CACHE_END; ++i )
		memoryGroups[ i ] = PlCreateMemoryGroup();

	InitializeCachePools();

	mmReferenceList = PlCreateLinkedList();
	if ( mmReferenceList == NULL )
		PRINT_ERROR( "Failed to create memory manager linked list!\n" );

	Sch_PushTask( MEM_CLEANUP_TASK_NAME, MEM_CB_Cleanup, NULL, MEM_CLEANUP_DELAY );
}

void MemoryManager_Shutdown( void )
{
	MemoryManager_FlushUnreferencedResources();

	unsigned int danglingReferences = PlGetNumLinkedListNodes( mmReferenceList );
	if ( danglingReferences > 0 )
		PRINT_WARNING( "Shutting down memory manager with %u dangling references!\n", danglingReferences );

	for ( unsigned int i = 0; i < MEM_CACHE_END; ++i )
		PlDestroyMemoryGroup( memoryGroups[ i ] );
}

unsigned int MemoryManager_FlushUnreferencedResources( void )
{
	unsigned int references = PlGetNumLinkedListNodes( mmReferenceList );
	while ( references > 0 )
	{
#if defined( DEBUG_MEMORY )
		DebugMsg( " dangling references: " COM_FMT_uint32 "\n", danglingReferences );
#endif

		CleanupUnreferencedResources( true );

		unsigned int n = PlGetNumLinkedListNodes( mmReferenceList );
		if ( n == references )
			break;

		references = n;
	}

	return references;
}

MMReference *MemoryManager_SetupReference( const char *id, uint8_t pool, MMReference *m, MMReference_CleanupFunction cleanupFunction, void *userData )
{
	if ( id != NULL )
		m->cache = MM_GetCachedData( id, pool );

	m->userData        = userData;
	m->cleanupFunction = cleanupFunction;
	m->isInitialized   = true;
	m->node            = PlInsertLinkedListNode( mmReferenceList, m );
	return m;
}

void MemoryManager_AddReference( MMReference *m )
{
	m->numReferences++;
	m->timeToLive = ( Engine_GetNumTicks() + 1024 );
#if defined( DEBUG_MEMORY )
	DebugMsg( "Adding reference: description(%s) numRefs(%d) ttl(%u)\n",
	          m->description[ 0 ] == '\0' ? "unknown" : m->description,
	          m->numRefs,
	          m->ttl );
#endif
}

void MemoryManager_ReleaseReference( MMReference *m )
{
	assert( m->numReferences > 0 );

#if defined( DEBUG_MEMORY )
	DebugMsg( "Releasing reference: description(%s) numRefs(%d) ttl(%u)\n",
	          m->description[ 0 ] == '\0' ? "unknown" : m->description,
	          m->numRefs,
	          m->ttl );
#endif

	m->numReferences--;
	if ( m->numReferences <= 0 )
	{
		m->timeToLive = ( Engine_GetNumTicks() + 1024 );
	}
}

int MemoryManager_GetNumberOfReferences( const MMReference *m )
{
	return m->numReferences;
}

/* ======================================================================
 * Temporary Buffer Allocation
 * ====================================================================*/

static void MEM_CB_CleanupTempAlloc( void *userData )
{
	PL_DELETE( userData );
}

/**
 * Allocates a pool of memory that will be automatically
 * cleaned up.
 */
void *MM_TempAlloc( MMReference *m, size_t size )
{
	void *buf = PlMAllocA( size );
	MemoryManager_SetupReference( "temp", 0, m, MEM_CB_CleanupTempAlloc, buf );
	return buf;
}

/**
 * Attempts to immediately free the given resource.
 * If this isn't called, resource will be cleaned
 * up automatically later.
 */
void MM_TempFree( MMReference *m )
{
	if ( !FreeReference( m, false ) )
	{
		PRINT_WARNING( "Failed to cleanup temporary pool!\n" );
	}
}
