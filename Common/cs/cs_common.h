/*
** 'C'-style script compiler
** Copyright (C) 2000-2001, Chris Jones
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE;
** the contents of this file may not be disclosed to third parties,
** copied or duplicated in any form, in whole or in part, without
** prior express permission from Chris Jones.
**
*/

#ifndef __CS_COMMON_H
#define __CS_COMMON_H

#define SCOM_VERSION 89
#define SCOM_VERSIONSTR "0.89"

struct ccScript
{
    char *globaldata;
    long globaldatasize;
    long *code;
    long codesize;
    char *strings;
    long stringssize;
    char *fixuptypes;             // global data/string area/ etc
    long *fixups;                 // code array index to fixup (in longs)
    int numfixups;
    int importsCapacity;
    char **imports;
    int numimports;
    int exportsCapacity;
    char **exports;   // names of exports
    long *export_addr;        // high byte is type; low 24-bits are offset
    int numexports;
    int instances;
    // 'sections' allow the interpreter to find out which bit
    // of the code came from header files, and which from the main file
    char **sectionNames;
    long *sectionOffsets;
    int numSections;
    int capacitySections;
};

#define SREG_SP           1     // stack pointer
#define SREG_MAR          2     // memory address register
#define SREG_AX           3     // general purpose
#define SREG_BX           4
#define SREG_CX           5
#define SREG_OP           6    // object pointer for member func calls
#define SREG_DX           7
#define CC_NUM_REGISTERS  8
#define INSTF_SHAREDATA   1
#define INSTF_ABORTED     2
#define INSTF_FREE        4
#define INSTF_RUNNING     8   // set by main code to confirm script isn't stuck
#define CC_STACK_SIZE     4000
#define MAX_CALL_STACK    100

struct ccInstance
{
    long flags;
    char *globaldata;
    long globaldatasize;
    unsigned long *code;
    ccInstance *runningInst;  // might point to another instance if in far call
    long codesize;
    char *strings;
    long stringssize;
    char **exportaddr;  // real pointer to export
    char *stack;
    long stacksize;
    long registers[CC_NUM_REGISTERS];
    long pc;                        // program counter
    long line_number;               // source code line number
    ccScript *instanceof;
    long callStackLineNumber[MAX_CALL_STACK];
    long callStackAddr[MAX_CALL_STACK];
    ccInstance *callStackCodeInst[MAX_CALL_STACK];
    int  callStackSize;
    int  loadedInstanceId;
    int  returnValue;
};

// virtual CPU commands
#define SCMD_ADD          1     // reg1 += arg2
#define SCMD_SUB          2     // reg1 -= arg2
#define SCMD_REGTOREG     3     // reg2 = reg1
#define SCMD_WRITELIT     4     // m[MAR] = arg2 (copy arg1 bytes)
#define SCMD_RET          5     // return from subroutine
#define SCMD_LITTOREG     6     // set reg1 to literal value arg2
#define SCMD_MEMREAD      7     // reg1 = m[MAR]
#define SCMD_MEMWRITE     8     // m[MAR] = reg1
#define SCMD_MULREG       9     // reg1 *= reg2
#define SCMD_DIVREG       10    // reg1 /= reg2
#define SCMD_ADDREG       11    // reg1 += reg2
#define SCMD_SUBREG       12    // reg1 -= reg2
#define SCMD_BITAND       13    // bitwise  reg1 & reg2
#define SCMD_BITOR        14    // bitwise  reg1 | reg2
#define SCMD_ISEQUAL      15    // reg1 == reg2   reg1=1 if true, =0 if not
#define SCMD_NOTEQUAL     16    // reg1 != reg2
#define SCMD_GREATER      17    // reg1 > reg2
#define SCMD_LESSTHAN     18    // reg1 < reg2
#define SCMD_GTE          19    // reg1 >= reg2
#define SCMD_LTE          20    // reg1 <= reg2
#define SCMD_AND          21    // (reg1!=0) && (reg2!=0) -> reg1
#define SCMD_OR           22    // (reg1!=0) || (reg2!=0) -> reg1
#define SCMD_CALL         23    // jump to subroutine at reg1
#define SCMD_MEMREADB     24    // reg1 = m[MAR] (1 byte)
#define SCMD_MEMREADW     25    // reg1 = m[MAR] (2 bytes)
#define SCMD_MEMWRITEB    26    // m[MAR] = reg1 (1 byte)
#define SCMD_MEMWRITEW    27    // m[MAR] = reg1 (2 bytes)
#define SCMD_JZ           28    // jump if ax==0 to arg1
#define SCMD_PUSHREG      29    // m[sp]=reg1; sp++
#define SCMD_POPREG       30    // sp--; reg1=m[sp]
#define SCMD_JMP          31    // jump to arg1
#define SCMD_MUL          32    // reg1 *= arg2
#define SCMD_CALLEXT      33    // call external (imported) function reg1
#define SCMD_PUSHREAL     34    // push reg1 onto real stack
#define SCMD_SUBREALSTACK 35
#define SCMD_LINENUM      36    // debug info - source code line number
#define SCMD_CALLAS       37    // call external script function
#define SCMD_THISBASE     38    // current relative address
#define SCMD_NUMFUNCARGS  39    // number of arguments for ext func call
#define SCMD_MODREG       40    // reg1 %= reg2
#define SCMD_XORREG       41    // reg1 ^= reg2
#define SCMD_NOTREG       42    // reg1 = !reg1
#define SCMD_SHIFTLEFT    43    // reg1 = reg1 << reg2
#define SCMD_SHIFTRIGHT   44    // reg1 = reg1 >> reg2
#define SCMD_CALLOBJ      45    // next call is member function of reg1
#define SCMD_CHECKBOUNDS  46    // check reg1 is between 0 and arg2
#define SCMD_MEMWRITEPTR  47    // m[MAR] = reg1 (adjust ptr addr)
#define SCMD_MEMREADPTR   48    // reg1 = m[MAR] (adjust ptr addr)
#define SCMD_MEMZEROPTR   49    // m[MAR] = 0    (blank ptr)
#define SCMD_MEMINITPTR   50    // m[MAR] = reg1 (but don't free old one)
#define SCMD_LOADSPOFFS   51    // MAR = SP - arg1 (optimization for local var access)
#define SCMD_CHECKNULL    52    // error if MAR==0
#define SCMD_FADD         53    // reg1 += arg2 (float,int)
#define SCMD_FSUB         54    // reg1 -= arg2 (float,int)
#define SCMD_FMULREG      55    // reg1 *= reg2 (float)
#define SCMD_FDIVREG      56    // reg1 /= reg2 (float)
#define SCMD_FADDREG      57    // reg1 += reg2 (float)
#define SCMD_FSUBREG      58    // reg1 -= reg2 (float)
#define SCMD_FGREATER     59    // reg1 > reg2 (float)
#define SCMD_FLESSTHAN    60    // reg1 < reg2 (float)
#define SCMD_FGTE         61    // reg1 >= reg2 (float)
#define SCMD_FLTE         62    // reg1 <= reg2 (float)
#define SCMD_ZEROMEMORY   63    // m[MAR]..m[MAR+(arg1-1)] = 0
#define SCMD_CREATESTRING 64    // reg1 = new String(reg1)
#define SCMD_STRINGSEQUAL 65    // (char*)reg1 == (char*)reg2   reg1=1 if true, =0 if not
#define SCMD_STRINGSNOTEQ 66    // (char*)reg1 != (char*)reg2
#define SCMD_CHECKNULLREG 67    // error if reg1 == NULL
#define SCMD_LOOPCHECKOFF 68    // no loop checking for this function
#define SCMD_MEMZEROPTRND 69    // m[MAR] = 0    (blank ptr, no dispose if = ax)
#define SCMD_JNZ          70    // jump to arg1 if ax!=0
#define SCMD_DYNAMICBOUNDS 71   // check reg1 is between 0 and m[MAR-4]
#define SCMD_NEWARRAY     72    // reg1 = new array of reg1 elements, each of size arg2 (arg3=managed type?)


#endif // __CS_COMMON_H