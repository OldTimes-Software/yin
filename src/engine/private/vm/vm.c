/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include "engine_private.h"
#include "vm.h"

static PLLinkedList *vmPrograms;

#define VM_MAX_ADDRESS     16384
#define VM_MAX_NAME_LENGTH 16

typedef uint64_t VMRegister;
typedef uint64_t VMAddress;

typedef struct VMProgram
{
	PLFile     *file;
	const char *filePath;

	VMRegister registers[ VM_MAX_REGISTERS ];
	VMAddress  memory[ VM_MAX_ADDRESS ];

	char name[ VM_MAX_NAME_LENGTH ];

	bool isRunning; /* whether or not the program should be ticked */

	unsigned int        lastTick;        /* last time we ticked */
	unsigned int        numTicks;        /* number of ticks total */
	unsigned int        curInstruction;  /* current instruction */
	unsigned int        numInstructions; /* total number of instructions */
	YANG_VMInstruction *instructions;    /* array of instructions */

	PLLinkedListNode *node; /* instance in the linked list */
} VMProgram;

typedef struct VMFunctionExport
{
	const char  *functionName;
	unsigned int numArguments;
	bool ( *Callback )( VMProgram *program );
} VMFunctionExport;

#define VM_FUNCTION_EXPORT( NAME, NUM_ARGS )                                      \
	static bool             VFE_##NAME( VMProgram *program );                     \
	static VMFunctionExport VFE_##NAME##_S = { #NAME, ( NUM_ARGS ), VFE_##NAME }; \
	static bool             VFE_##NAME( VMProgram *program )

/**
 * Prints a message. Requires a message to be passed.
 */
VM_FUNCTION_EXPORT( printf, -1 )
{
	PRINT( "Print called!\n" );
	return false;
}

/**
 * Throws a warning. Requires a message to be passed.
 */
VM_FUNCTION_EXPORT( Warning, -1 )
{
	PRINT_WARNING( "Warning called!\n" );
	return false;
}

/**
 * Throws an error. Requires a message to be passed.
 */
VM_FUNCTION_EXPORT( Error, -1 )
{
	PRINT_ERROR( "Error called!\n" );
}

/**
 * Kills the currently active program.
 */
VM_FUNCTION_EXPORT( Abort, 0 )
{
	abort();
}

const VMFunctionExport *vmFunctionExports[] = {
		&VFE_printf_S,
		&VFE_Warning_S,
		&VFE_Error_S,
		&VFE_Abort_S,
};

/*--------------------------
 * Memory Management
 * */

static void VM_ClearMemory( VMProgram *program )
{
	memset( program->memory, 0, sizeof( VMAddress ) * VM_MAX_ADDRESS );
}

static void VM_WriteMemory( VMProgram *program, uint32_t address, uint32_t value )
{
	if ( address >= VM_MAX_ADDRESS )
		PRINT_ERROR( "Attempted to write \"%d\" to invalid address \"%d\"!\n", value, address );

	program->memory[ address ] = value;
}

static VMAddress VM_ReadMemory( const VMProgram *program, uint32_t address )
{
	if ( address >= VM_MAX_ADDRESS )
		PRINT_ERROR( "Attempted to read from invalid address \"%d\"!\n", address );

	return program->memory[ address ];
}

/*--------------------------
 * */

VMProgram *VM_GetProgramByName( const char *programName )
{
	PLLinkedListNode *curNode = PlGetFirstNode( vmPrograms );
	while ( curNode != NULL )
	{
		VMProgram *program = ( VMProgram * ) PlGetLinkedListNodeUserData( curNode );
		if ( strcmp( program->name, programName ) == 0 )
			return program;

		curNode = PlGetNextLinkedListNode( curNode );
	}

	PRINT_WARNING( "Failed to find the specified VM program!\n" );

	return NULL;
}

void VM_ExecuteProgram( VMProgram *program )
{
	program->isRunning = true;
}

static VMProgram *VM_ParseProgram( PLFile *file )
{
	const char *filePath = PlGetFilePath( file );
	uint32_t    magic = PlReadInt32( file, false, NULL );
	if ( magic != YANG_EXE_MAGIC )
	{
		PRINT_WARNING( "Unexpected magic for \"%s\"!\n", filePath );
		return NULL;
	}

	uint16_t version = PlReadInt16( file, false, NULL );
	if ( version == 0 || version > YANG_EXE_VERSION )
	{
		PRINT_WARNING( "Unexpected version for \"%s\"!\n", filePath );
		return NULL;
	}

	uint16_t numSections = PlReadInt16( file, false, NULL );
	if ( numSections == 0 || numSections >= YANG_EXE_MAX_SECTIONS )
	{
		PRINT_WARNING( "Unexpected number of sections for \"%s\"!\n", filePath );
		return NULL;
	}

	/* read in all of the instructions */
	bool                status;
	unsigned int        numInstructions = PlReadInt32( file, false, &status );
	YANG_VMInstruction *instructions = calloc( numInstructions, sizeof( YANG_VMInstruction ) );
	if ( instructions == NULL )
		PRINT_ERROR( "Failed to allocate instruction buffer!\n" );

	for ( unsigned int i = 0; i < numInstructions; ++i )
		instructions[ i ].opCode = PlReadInt8( file, &status );

	if ( !status )
		PRINT_ERROR( "Failed to read in instructions for \"%s\"!\nPL: %s\n", filePath, PlGetError() );

	/* now we can actually setup the VM */

	VMProgram *program = calloc( 1, sizeof( VMProgram ) );
	if ( program == NULL )
		PRINT_ERROR( "Failed to allocate program!\n" );

	program->numInstructions = numInstructions;
	program->instructions = instructions;
	program->node = PlInsertLinkedListNode( vmPrograms, program );

	return program;
}

VMProgram *VM_LoadProgram( const char *path )
{
	PLFile *filePtr = PlOpenFile( path, true );
	if ( filePtr == NULL )
	{
		PRINT_WARNING( "Failed to open YEXE, \"%s\"!\nPL: %s\n", path, PlGetError() );
		return NULL;
	}

	/* validate it */

	VMProgram *program = VM_ParseProgram( filePtr );

	PlCloseFile( filePtr );

	return program;
}

static void VM_Evaluate( VMProgram *vmHandle, YANG_VMInstruction *curInstruction )
{
	switch ( curInstruction->opCode )
	{
		default:
			PRINT_WARNING( "Invalid opcode \"%d\" encountered! "
			           "VM state will probably be corrupted\n" );
			break;
		case VM_OP_NOP:
			break;
		//case VM_OP_ADD_I:
		//	break;
		//case VM_OP_MUL_I:
		//	break;
		//case VM_OP_NEG_I:
		//	break;
		case VM_OP_RETURN:
			break;
	}
}

static void VM_TickProgram( VMProgram *program )
{
	if ( !program->isRunning )
		return;

	YANG_VMInstruction *curInstruction = &program->instructions[ program->curInstruction++ ];
	VM_Evaluate( program, curInstruction );

	if ( program->curInstruction >= program->numInstructions )
	{
		PRINT_ERROR( "Overrun in program, \"%s\"!\n", program->name );
	}
}

void VM_TerminateProgram( VMProgram *program )
{
}

static void VM_SetClockSpeed( unsigned int argc, char **argv )
{
}

static void VM_FreezeCallback( unsigned int argc, char **argv )
{
	if ( argc <= 1 )
	{
		PRINT_WARNING( "Invalid arguments!\n" );
		return;
	}

	const char *programName = argv[ 1 ];
	VMProgram  *program = VM_GetProgramByName( programName );
	if ( program == NULL )
		return;

	program->isRunning = false;
}

static void VM_TerminateCallback( unsigned int argc, char **argv )
{
}

static void VM_ExecuteCallback( unsigned int argc, char **argv )
{
}

static void VM_AssembleCallback( unsigned int argc, char **argv )
{
	if ( argc <= 2 )
	{
		PRINT_WARNING( "Invalid arguments!\n" );
		return;
	}

	const char *asmPath = argv[ 1 ];
	const char *outPath = argv[ 2 ];

	PRINT( "Assembling \"%s\"...\n", asmPath );

	PLFile *filePtr = PlOpenLocalFile( asmPath, false );
	if ( filePtr == NULL )
	{
		PRINT_WARNING( "Failed to open \"%s\"!\nPL: %s\n", asmPath, PlGetError() );
		return;
	}

	PRINT( "Wrote \"%s\"\n" );
}

void VM_Initialize( void )
{
	PRINT( "Initializing Virtual Machine...\n" );

	PlRegisterConsoleCommand( "Vm.SetClockSpeed", "Set the clock speed of the specified program.", 1, VM_SetClockSpeed );
	PlRegisterConsoleCommand( "Vm.Freeze", "Freeze the specified program.", 1, VM_FreezeCallback );
	PlRegisterConsoleCommand( "Vm.Terminate", "Terminate the specified program.", 1, VM_TerminateCallback );
	PlRegisterConsoleCommand( "Vm.Execute", "Execute the specified program.", 1, VM_ExecuteCallback );
	PlRegisterConsoleCommand( "Vm.Assemble", "Assembles the specified ASM.", 1, VM_AssembleCallback );

	vmPrograms = PlCreateLinkedList();
	if ( vmPrograms == NULL )
		PRINT_ERROR( "Failed to create vmPrograms list!\nPL: %s\n", PlGetError() );

	//VM_LoadCVM
}

void VM_Shutdown( void )
{
	PlDestroyLinkedList( vmPrograms );
}
