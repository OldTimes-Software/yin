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

#include <plcore/pl.h>
#include <plcore/pl_filesystem.h>

#ifdef L_PROFILE
#	include "timing.h"
#endif

#define dprintf( ... ) printf( __VA_ARGS__ )
#define dgets( A, B )  fgets( ( A ), ( B ), stdin )
#define dputc( A )     fputc( ( A ), stdin )

#define Cell               void
#define MAX_LISP_TOKEN_LEN 200

#define NILP( x )     ( ( x ) == nullptr )
#define DEFINEDP( x ) ( ( x ) != l_undefined )

enum
{
	L_BAD_CELL,// error catching type
	L_CONS_CELL,
	L_NUMBER,
	L_SYMBOL,
	L_SYS_FUNCTION,
	L_USER_FUNCTION,
	L_STRING,
	L_CHARACTER,
	L_C_FUNCTION,
	L_C_BOOL,
	L_L_FUNCTION,
	L_POINTER,
	L_OBJECT_VAR,
	L_1D_ARRAY,
	L_FIXED_POINT,
	L_COLLECTED_OBJECT,
};

// FIXME: switch this to uint8_t one day? it still breaks stuff
typedef uint8_t ltype;

typedef intptr_t ( *LSysCallback )( void * );

#if defined( __cplusplus )
struct LSpace
{
	size_t GetFree() const;
	void  *Alloc( size_t size );

	void *Mark() const;
	void  Restore( void *val );
	void  Clear();

	static LSpace  Tmp, Perm, Gc;
	static LSpace *Current;

	uint8_t    *m_data;
	uint8_t    *m_free;
	char const *m_name;
	size_t      m_size;
};

struct LObject
{
	/* Factories */
	static LObject *Compile( char const *&s );

	/* Methods */
	LObject *Eval();
	void     Print();

	/* Members */
	ltype m_type;
};

struct LObjectVar : LObject
{
	/* Factories */
	static LObjectVar *Create( int index );

	/* Members */
	int m_index;
};

struct LList : LObject
{
	/* Factories */
	static LList *Create();

	/* Methods */
	size_t GetLength();
	LList *Assoc( LObject *item );

	/* Members */
	LObject *m_cdr, *m_car;
};

struct LNumber : LObject
{
	/* Factories */
	static LNumber *Create( long num );

	/* Members */
	long m_num;
};

struct LRedirect : LObject
{
	/* Members */
	LObject *m_ref;
};

struct LString : LObject
{
	/* Factories */
	static LString *Create( char const *string );
	static LString *Create( char const *string, int length );
	static LString *Create( int length );

	/* Methods */
	char *GetString();

	/* Members */
private:
	char m_str[ 1 ]; /* Can be allocated much larger than 1 */
};

struct LSymbol : LObject
{
	/* Factories */
	static LSymbol *Find( char const *name );
	static LSymbol *FindOrCreate( char const *name );

	/* Methods */
	LObject *EvalFunction( void *arg_list );
	LObject *EvalUserFunction( LList *arg_list );

	LString *GetName();
	LObject *GetFunction();
	LObject *GetValue();

	void SetFunction( LObject *fun );
	void SetValue( LObject *value );
	void SetNumber( long num );

	/* Members */
#	ifdef L_PROFILE
	double time_taken;
#	endif
	LObject *m_value;
	LObject *m_function;
	LString *m_name;
	LSymbol *m_left, *m_right;// tree structure

	/* Static members */
	static LSymbol *root;
	static size_t   count;
};

struct LSysFunction : LObject
{
	/* Methods */
	LObject *EvalFunction( LList *arg_list );

	/* Members */
	short min_args, max_args;
	LSysCallback callback;
};

struct LUserFunction : LObject
{
	LList *arg_list, *block_list;
};

struct LArray : LObject
{
	/* Factories */
	static LArray *Create( size_t len, void *rest );

	/* Methods */
	inline LObject **GetData() { return m_data; }
	LObject         *Get( int x );

	/* Members */
	size_t m_len;

private:
	LObject *m_data[ 1 ]; /* Can be allocated much larger than 1 */
};

struct LChar : LObject
{
	/* Factories */
	static LChar *Create( uint16_t ch );

	/* Methods */
	uint16_t GetValue();

	/* Members */
	uint16_t m_ch;
};

struct LPointer : LObject
{
	/* Factories */
	static LPointer *Create( void *addr );

	/* Members */
	void *m_addr;
};

struct LFixedPoint : LObject
{
	/* Factories */
	static LFixedPoint *Create( int32_t x );

	/* Members */
	int32_t m_fixed;
};

class Lisp
{
public:
	static void Init();
	static void Uninit();

	static void InitConstants();

	// Collect temporary or permanent spaces
	static void CollectSpace( LSpace *which_space, int grow );

private:
	static LArray  *CollectArray( LArray *x );
	static LList   *CollectList( LList *x );
	static LObject *CollectObject( LObject *x );
	static void     CollectSymbols( LSymbol *root );
	static void     CollectStacks();
};
#else
typedef struct LSpace        LSpace;
typedef struct LObject       LObject;
typedef struct LObjectVar    LObjectVar;
typedef struct LList         LList;
typedef struct LNumber       LNumber;
typedef struct LRedirect     LRedirect;
typedef struct LString       LString;
typedef struct LSymbol       LSymbol;
typedef struct LSysFunction  LSysFunction;
typedef struct LUserFunction LUserFunction;
typedef struct LArray        LArray;
typedef struct LChar         LChar;
typedef struct LPointer      LPointer;
typedef struct LFixedPoint   LFixedPoint;
#endif

#if defined( __GNUC__ ) && !defined( __clang__ )
/*
 * C++ spec says "this" is always NON-NULL, recent versions of gcc will warn
 * about this and optimizes the "if (this)" we use in some places away:
 * "warning: nonnull argument ‘this’ compared to NULL [-Wnonnull-compare]"
 * We rely on "if (this)" checks in several places and refactoring this is
 * non trivial. So we use this little helper marked with
 * __attribute__((optimize("O0"))) to workaround this.
 */
static inline bool __attribute__( ( optimize( "O0" ) ) ) ptr_is_null( void *ptr )
{
	return ptr == NULL;
}
#else
static inline bool           ptr_is_null( void *ptr )
{
	return ptr == NULL;
}
#endif

PL_EXTERN_C

static inline ltype item_type( void *x )
{
	if ( !ptr_is_null( x ) ) return *( ltype * ) x;
	return L_CONS_CELL;
}

void    *lpointer_value( void *lpointer );
int32_t  lnumber_value( void *lnumber );
long     lfixed_point_value( void *c );
void    *lisp_atom( void *i );
LObject *lcdr( void *c );
LObject *lcar( void *c );
void    *lisp_eq( void *n1, void *n2 );
void    *lisp_equal( void *n1, void *n2 );
void    *eval_block( void *list );

LSymbol *add_c_object( void *symbol, int index );
LSymbol *add_c_function( char const *name, short min_args, short max_args, long ( *Callback )( void * ) );
LSymbol *add_c_bool_fun( char const *name, short min_args, short max_args, short number );
LSymbol *add_lisp_function( char const *name, short min_args, short max_args, short number );
void     print_trace_stack( int max_levels );

LSysFunction *new_lisp_sys_function( int min_args, int max_args, LSysCallback Callback );
LSysFunction *new_lisp_c_function( int min_args, int max_args, LSysCallback Callback );
LSysFunction *new_lisp_c_bool( int min_args, int max_args, LSysCallback Callback );

LUserFunction *new_lisp_user_function( LList *arg_list, LList *block_list );

LSysFunction *new_user_lisp_function( int min_args, int max_args, LSysCallback Callback );

void   *nth( int num, void *list );
int32_t lisp_atan2( int32_t dy, int32_t dx );
int32_t lisp_sin( int32_t x );
int32_t lisp_cos( int32_t x );

void lbreak( const char *format, ... );

// FIXME: get rid of this later
LObject *symbol_value( void *sym );
char    *lstring_value( void *str );

#include "lisp_opt.h"

/* C interface for Lisp API */

void LspInit( void );
void LspUninit( void );
void LspCollectSpace( LSpace *whichSpace, int grow );

void LspPermSpace();
void LspTmpSpace();

/*****************************************
 * LObject
 *****************************************
 */

LObject *LspCAR( void *x );
LObject *LspCDR( void *x );

LObject *LspCompileObject( const char *s );
LObject *LspEvalObject( LObject *object );

/*****************************************
 * LSpace
 *****************************************
 */

LSpace *LspGetTempSpace( void );
LSpace *LspGetPermSpace( void );

size_t LspGetSpaceFree( LSpace *space );
void  *LspAllocSpace( LSpace *space, size_t size );
void   LspClearSpace( LSpace *space );

PL_EXTERN_C_END
