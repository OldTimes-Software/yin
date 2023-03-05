/*
 *  Abuse - dark 2D side-scrolling platform game
 *  Copyright (c) 2005-2011 Sam Hocevar <sam@hocevar.net>
 *  Copyright (c) 2022 Mark Sowden <hogsy@oldtimes-software.com>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Sam Hocevar.
 */

struct func
{
	char const  *name;
	short        min_args, max_args;
	LSysCallback callback;
};

/* select, digistr, load-file are not common lisp functions! */

static intptr_t PrintFunction( void *args )
{
	LObject *ret = NULL;
	LList   *argList = ( LList   *) args;
	while ( argList )
	{
		ret = CAR( argList )->Eval();
		argList = ( LList * ) CDR( argList );
		ret->Print();
	}
	return ( intptr_t ) ret;
}

static intptr_t CARFunction( void *args )
{
	return ( intptr_t ) lcar( CAR( ( LList * ) args )->Eval() );
}

static intptr_t CDRFunction( void *args )
{
	return ( intptr_t ) lcdr( CDR( ( LList * ) args )->Eval() );
}

static intptr_t LengthFunction( void *args )
{
	LObject *v = CAR( ( LList * ) args )->Eval();
	switch ( item_type( v ) )
	{
		case L_STRING:
			return ( intptr_t ) LNumber::Create( ( long ) strlen( lstring_value( v ) ) );
		case L_CONS_CELL:
			return ( intptr_t ) LNumber::Create( ( long ) ( ( LList * ) v )->GetLength() );
		default:
			v->Print();
			lbreak( "length : type not supported\n" );
			break;
	}

	return 0;
}

static intptr_t ListFunction( void *args )
{
	LList *cur = nullptr, *last = nullptr, *first = nullptr;
	LList *arg_list = ( LList * ) args;
	PtrRef r1( cur ), r2( first ), r3( last );
	while ( arg_list )
	{
		cur = LList::Create();
		LObject *val = CAR( arg_list )->Eval();
		cur->m_car = val;
		if ( last )
			last->m_cdr = cur;
		else
			first = cur;
		last = cur;
		arg_list = ( LList * ) CDR( arg_list );
	}
	return ( intptr_t ) first;
}

static intptr_t EqualFunction( void *args )
{
	LList *arg_list = ( LList * ) args;
	l_user_stack.push( CAR( arg_list )->Eval() );
	l_user_stack.push( CAR( CDR( arg_list ) )->Eval() );
	return ( intptr_t ) lisp_equal( l_user_stack.pop( 1 ), l_user_stack.pop( 1 ) );
}

static intptr_t PlusFunction( void *args )
{
	LList  *arg_list = ( LList  *) args;
	int32_t sum = 0;
	while ( arg_list )
	{
		sum += lnumber_value( CAR( arg_list )->Eval() );
		arg_list = ( LList * ) CDR( arg_list );
	}
	return ( intptr_t ) LNumber::Create( sum );
}

static intptr_t AbsFunction( void *args )
{
	return ( intptr_t ) LNumber::Create( abs( lnumber_value( CAR( ( LList * ) args )->Eval() ) ) );
}

static int      end_of_program( char const *s );
static intptr_t LoadFunction( void *args )
{
	LList   *arg_list = ( LList   *) args;
	LObject *fn = CAR( arg_list )->Eval();
	PtrRef   r1( fn );
	char    *st = lstring_value( fn );

	PLFile *fp = PlOpenFile( st, false );
	if ( fp == nullptr )
	{
		if ( DEFINEDP( ( ( LSymbol * ) load_warning )->GetValue() ) && ( ( LSymbol * ) load_warning )->GetValue() )
			dprintf( "Warning : file %s does not exist\n", st );

		return 0;
	}

	size_t l = PlGetFileSize( fp );
	char  *s = ( char  *) PL_NEW_( char, l + 1 );

	PlReadFile( fp, s, sizeof( char ), l );
	s[ l ] = 0;
	PlCloseFile( fp );
	char const *cs = s;

	LObject *compiled_form = nullptr;
	PtrRef   r11( compiled_form );
	while ( !end_of_program( cs ) )// see if there is anything left to compile and run
	{
		void *m = LSpace::Tmp.Mark();
		compiled_form = LObject::Compile( cs );
		compiled_form->Eval();
		compiled_form = nullptr;
		LSpace::Tmp.Restore( m );
	}

	PL_DELETE( s );
	return ( intptr_t ) fn;
}

static intptr_t OpenFileFunction( void *args )
{
	LList   *arg_list = ( LList   *) args;
	LObject *str1 = CAR( arg_list )->Eval();
	PtrRef   r1( str1 );
	LObject *str2 = CAR( CDR( arg_list ) )->Eval();

	LObject *ret = nullptr;

	FILE *old_file = current_print_file;
	current_print_file = fopen( lstring_value( str1 ), lstring_value( str2 ) );
	if ( current_print_file != nullptr )
	{
		while ( arg_list )
		{
			ret = CAR( arg_list )->Eval();
			arg_list = ( LList * ) CDR( arg_list );
		}
	}
	fclose( current_print_file );
	current_print_file = old_file;

	return ( intptr_t ) ret;
}

static intptr_t Num2StrFunction( void *args )
{
	char str[ 20 ];
	sprintf( str, "%ld", ( long int ) lnumber_value( CAR( ( LList * ) args )->Eval() ) );
	return ( intptr_t ) LString::Create( str );
}

struct func sys_funcs[] =
        {
                { "print", 1, -1, PrintFunction },        /* 0 */
                { "car", 1, 1, CARFunction },             /* 1 */
                { "cdr", 1, 1, CDRFunction },             /* 2 */
                { "length", 0, -1, LengthFunction },      /* 3 */
                { "list", 0, -1, ListFunction },          /* 4 */
                { "cons", 2, 2 },                         /* 5 */
                { "quote", 1, 1 },                        /* 6 */
                { "eq", 2, 2, EqualFunction },            /* 7 */
                { "+", 0, -1, PlusFunction },             /* 8 */
                { "-", 1, -1 },                           /* 9 */
                { "if", 2, 3 },                           /* 10 */
                { "setf", 2, 2 },                         /* 11 */
                { "symbol-list", 0, 0 },                  /* 12 */
                { "assoc", 2, 2 },                        /* 13 */
                { "null", 1, 1 },                         /* 14 */
                { "acons", 2, 2 },                        /* 15 */
                { "pairlis", 2, 2 },                      /* 16 */
                { "let", 1, -1 },                         /* 17 */
                { "defun", 2, -1 },                       /* 18 */
                { "atom", 1, 1 },                         /* 19 */
                { "not", 1, 1 },                          /* 20 */
                { "and", -1, -1 },                        /* 21 */
                { "or", -1, -1 },                         /* 22 */
                { "progn", -1, -1 },                      /* 23 */
                { "equal", 2, 2 },                        /* 24 */
                { "concatenate", 1, -1 },                 /* 25 */
                { "char-code", 1, 1 },                    /* 26 */
                { "code-char", 1, 1 },                    /* 27 */
                { "*", -1, -1 },                          /* 28 */
                { "/", 1, -1 },                           /* 29 */
                { "cond", -1, -1 },                       /* 30 */
                { "select", 1, -1 },                      /* 31 */
                { "function", 1, 1 },                     /* 32 */
                { "mapcar", 2, -1 },                      /* 33 */
                { "funcall", 1, -1 },                     /* 34 */
                { ">", 2, 2 },                            /* 35 */
                { "<", 2, 2 },                            /* 36 */
                { "tmp-space", 0, 0 },                    /* 37 */
                { "perm-space", 0, 0 },                   /* 38 */
                { "symbol-name", 1, 1 },                  /* 39 */
                { "trace", 0, -1 },                       /* 40 */
                { "untrace", 0, -1 },                     /* 41 */
                { "digstr", 2, 2 },                       /* 42 */
                { "compile-file", 1, 1, LoadFunction },   /* 43 */
                { "abs", 1, 1, AbsFunction },             /* 44 */
                { "min", 2, 2 },                          /* 45 */
                { "max", 2, 2 },                          /* 46 */
                { ">=", 2, 2 },                           /* 47 */
                { "<=", 2, 2 },                           /* 48 */
                { "backquote", 1, 1 },                    /* 49 */
                { "comma", 1, 1 },                        /* 50 */
                { "nth", 2, 2 },                          /* 51 */
                { "cos", 1, 1 },                          /* 54 */
                { "sin", 1, 1 },                          /* 55 */
                { "atan2", 2, 2 },                        /* 56 */
                { "enum", 1, -1 },                        /* 57 */
                { "quit", 0, 0 },                         /* 58 */
                { "eval", 1, 1 },                         /* 59 */
                { "break", 0, 0 },                        /* 60 */
                { "mod", 2, 2 },                          /* 61 */
                { "write_profile", 1, 1 },                /* 62 */
                { "setq", 2, 2 },                         /* 63 */
                { "for", 4, -1 },                         /* 64 */
                { "open_file", 2, -1, OpenFileFunction }, /* 65 */
                { "load", 1, 1, LoadFunction },           /* 66 */
                { "bit-and", 1, -1 },                     /* 67 */
                { "bit-or", 1, -1 },                      /* 68 */
                { "bit-xor", 1, -1 },                     /* 69 */
                { "make-array", 1, -1 },                  /* 70 */
                { "aref", 2, 2 },                         /* 71 */
                { "if-1progn", 2, 3 },                    /* 72 */
                { "if-2progn", 2, 3 },                    /* 73 */
                { "if-12progn", 2, 3 },                   /* 74 */
                { "eq0", 1, 1 },                          /* 75 */
                { "preport", 1, 1 },                      /* 76 */
                { "search", 2, 2 },                       /* 77 */
                { "elt", 2, 2 },                          /* 78 */
                { "listp", 1, 1 },                        /* 79 */
                { "numberp", 1, 1 },                      /* 80 */
                { "do", 2, 3 },                           /* 81 */
                { "gc", 0, 0 },                           /* 82 */
                { "schar", 2, 2 },                        /* 83 */
                { "symbolp", 1, 1 },                      /* 84 */
                { "num2str", 1, 1, Num2StrFunction },     /* 85 */
                { "nconc", 2, -1 },                       /* 86 */
                { "first", 1, 1 },                        /* 87 */
                { "second", 1, 1 },                       /* 88 */
                { "third", 1, 1 },                        /* 89 */
                { "fourth", 1, 1 },                       /* 90 */
                { "fifth", 1, 1 },                        /* 91 */
                { "sixth", 1, 1 },                        /* 92 */
                { "seventh", 1, 1 },                      /* 93 */
                { "eighth", 1, 1 },                       /* 94 */
                { "ninth", 1, 1 },                        /* 95 */
                { "tenth", 1, 1 },                        /* 96 */
                { "substr", 3, 3 },                       /* 97 */
                { "local_load", 1, 1, LoadFunction },     /* 98 */
};
