/*
 *  Copyright (c) 2022 Mark Sowden <hogsy@oldtimes-software.com>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#include "lisp_private.h"

extern "C" LObject *symbol_value( void *sym ) { return ( ( LSymbol * ) sym )->GetValue(); }
extern "C" char    *lstring_value( void *str ) { return ( ( LString    *) str )->GetString(); }

extern "C" void LspInit( void ) { Lisp::Init(); }
extern "C" void LspUninit( void ) { Lisp::Uninit(); }
extern "C" void LspCollectSpace( LSpace *whichSpace, int grow ) { Lisp::CollectSpace( whichSpace, grow ); }

extern "C" LObject *LspCAR( void *x ) { return CAR( x ); }
extern "C" LObject *LspCDR( void *x ) { return CDR( x ); }
extern "C" LObject *LspCompileObject( const char *s ) { return LObject::Compile( s ); }
extern "C" LObject *LspEvalObject( LObject *object ) { return object->Eval(); }

extern "C" LSpace *LspGetTempSpace( void ) { return &LSpace::Tmp; }
extern "C" LSpace *LspGetPermSpace( void ) { return &LSpace::Perm; }

extern "C" size_t LspGetSpaceFree( LSpace *space ) { return space->GetFree(); }
extern "C" void  *LspAllocSpace( LSpace *space, size_t size ) { return space->Alloc( size ); }
extern "C" void   LspClearSpace( LSpace *space ) { return space->Clear(); }
