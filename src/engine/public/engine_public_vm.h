/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include "common.h"

#define YANG_TAG_MAX_LENGTH 64

typedef enum YANG_IntegralType
{
	VM_INTEGRALTYPE_FLOAT,
	VM_INTEGRALTYPE_CHAR,
	VM_INTEGRALTYPE_UCHAR,
	VM_INTEGRALTYPE_SHORT,
	VM_INTEGRALTYPE_USHORT,
	VM_INTEGRALTYPE_INT,
	VM_INTEGRALTYPE_UINT,
	VM_INTEGRALTYPE_STRING,

	YANG_MAX_INTEGRAL_TYPES
} YANG_IntegralType;

typedef enum VMRegisterType
{
	VM_REG_0,
	VM_REG_1,
	VM_REG_2,
	VM_REG_3,
	VM_REG_4,
	VM_REG_5,
	VM_REG_6,
	VM_REG_7,
	VM_REG_PC,
	VM_REG_COND,
	VM_REG_COUNT,

	VM_MAX_REGISTERS
} VMRegisterType;

typedef enum VMOpCode
{
	VM_OP_NOP, /* invalid instruction, throws error */
	VM_OP_RETURN,
	VM_OP_OR,
	VM_OP_AND,
	VM_OP_CALL,
	VM_OP_LOAD,

	// 32-bit integer opcodes
	VM_OP_MUL_I32,
	VM_OP_INC_I32,
	VM_OP_ADD_I32,
	VM_OP_SUB_I32,
	VM_OP_NEG_I32,
	// 32-bit floating-point opcodes
	VM_OP_MUL_F32,
	VM_OP_INC_F32,
	VM_OP_ADD_F32,
	VM_OP_SUB_F32,
	VM_OP_NEG_F32,

	VM_MAX_OPCODES
} VMOpCode;

enum
{
	PL_BITFLAG( VM_FL_POS, 0 ),
	PL_BITFLAG( VM_FL_ZRO, 1 ),
	PL_BITFLAG( VM_FL_NEG, 2 ),
};

typedef struct YANG_VMString
{
	char  *buf;
	size_t length;
} YANG_VMString;

typedef union VMDataFormatUnion
{
	uint8_t  ui8;
	int8_t   i8;
	uint16_t ui16;
	int16_t  i16;
	uint32_t ui32;
	int32_t  i32;
	float    f32;

	YANG_VMString string;
} VMDataFormatUnion;

typedef struct YANG_VMInstruction
{
	uint8_t           opCode;
	uint8_t           flag;
	VMDataFormatUnion data;
} YANG_VMInstruction;

typedef struct YANG_VMDataIndex
{
	uint8_t           type;
	VMDataFormatUnion data;
} YANG_VMDataIndex;

typedef struct YANG_VMFunction
{
	char     functionName[ YANG_TAG_MAX_LENGTH ];
	uint32_t offset;
} YANG_VMFunction;

/****************************************
 * Executable Format
 ****************************************/

#define YANG_EXE_EXTENSION ".yex"
#define YANG_EXE_MAGIC     PL_MAGIC_TO_NUM( 'Y', 'A', 'N', 'G' )
#define YANG_EXE_VERSION   1

typedef enum YANG_EXESection
{
	YANG_EXE_SECTION_DATA,
	YANG_EXE_SECTION_FUNCTION,
	YANG_EXE_SECTION_ENTRY,

	YANG_EXE_MAX_SECTIONS
} YANG_EXESection;

typedef struct YANG_EXESectionHeader
{
	uint8_t  type;
	uint32_t offset;
	uint32_t length;
} YANG_EXESectionHeader;

typedef struct YANG_EXEHeader
{
	char                  magic[ 4 ]; /* 'YANG' */
	uint16_t              version;
	uint16_t              numSections;
	YANG_EXESectionHeader sectionTable[ YANG_EXE_MAX_SECTIONS ];
} YANG_EXEHeader;
