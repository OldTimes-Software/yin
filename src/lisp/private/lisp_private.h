/*
 *  Abuse - dark 2D side-scrolling platform game
 *  Copyright (c) 1995 Crack dot Com
 *  Copyright (c) 2005-2011 Sam Hocevar <sam@hocevar.net>
 *  Copyright (c) 2022 Mark Sowden <hogsy@oldtimes-software.com>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#pragma once

#include "lisp.h"

static inline LObject *&CAR( void *x )
{
	return ( ( LList * ) x )->m_car;
}
static inline LObject *&CDR( void *x )
{
	return ( ( LList * ) x )->m_cdr;
}

void push_onto_list( void *object, void *&list );
