//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// 'C'-style script interpreter
//
//=============================================================================

#ifndef __CC_INSTANCE_H
#define __CC_INSTANCE_H

#include "script/script_common.h"
#include "script/cc_script.h"  // ccScript
#include "script/nonblockingscriptfunction.h"
#include "script/runtimescriptvalue.h"

namespace AGS { namespace Common { class DataStream; }; };

#define INSTF_SHAREDATA     1
#define INSTF_ABORTED       2
#define INSTF_FREE          4
#define INSTF_RUNNING       8   // set by main code to confirm script isn't stuck
#define CC_STACK_SIZE       1000
#define CC_STACK_DATA_SIZE  (1000 * sizeof(long))
#define MAX_CALL_STACK      100

// 256 because we use 8 bits to hold instance number
#define MAX_LOADED_INSTANCES 256

#define INSTANCE_ID_SHIFT 24
#define INSTANCE_ID_MASK  0x00000ff
#define INSTANCE_ID_REMOVEMASK 0x00ffffff

struct ccInstance;
struct ScriptImport;

struct ScriptInstruction
{
    ScriptInstruction()
    {
        Code		= 0;
        InstanceId	= 0;
    }

    long	Code;
    long	InstanceId;
};

struct ScriptOperation
{
	ScriptOperation()
	{
		ArgCount = 0;
	}

	ScriptInstruction   Instruction;
	RuntimeScriptValue	Args[MAX_SCMD_ARGS];
	int				    ArgCount;
};

// Running instance of the script
struct ccInstance
{
public:
    long flags;
    char *globaldata;
    long globaldatasize;
    unsigned long *code;
    ccInstance *runningInst;  // might point to another instance if in far call
    long codesize;
    char *strings;
    long stringssize;
    char **exportaddr;  // real pointer to export
    RuntimeScriptValue *stack;
    int  num_stackentries;
    // An array for keeping stack data; stack entries reference unknown data from here
    // TODO: probably change to dynamic array later
    char *stackdata;    // for storing stack data of unknown type
    char *stackdata_ptr;// works similar to original stack pointer, points to the next unused byte in stack data array
    long stackdatasize; // conventional size of stack data in bytes
    //
    RuntimeScriptValue registers[CC_NUM_REGISTERS];
    long pc;                        // program counter
    long line_number;               // source code line number
    ccScript *instanceof;
    long callStackLineNumber[MAX_CALL_STACK];
    long callStackAddr[MAX_CALL_STACK];
    ccInstance *callStackCodeInst[MAX_CALL_STACK];
    int  callStackSize;
    int  loadedInstanceId;
    int  returnValue;

    // array of real import indexes used in script
    int  *resolved_imports;
    int  numimports;

    char *code_fixups;

    // returns the currently executing instance, or NULL if none
    static ccInstance *GetCurrentInstance(void);
    // create a runnable instance of the supplied script
    static ccInstance *CreateFromScript(ccScript *script);
    static ccInstance *CreateEx(ccScript * scri, ccInstance * joined);

    ccInstance();
    ~ccInstance();
    // create a runnable instance of the same script, sharing global memory
    ccInstance *Fork();
    // specifies that when the current function returns to the script, it
    // will stop and return from CallInstance
    void    Abort();
    // aborts instance, then frees the memory later when it is done with
    void    AbortAndDestroy();
    
    // call an exported function in the script (2nd arg is number of params)
    int     CallScriptFunction(char *, long, ...);
    void    DoRunScriptFuncCantBlock(NonBlockingScriptFunction* funcToRun, bool *hasTheFunc);
    int     PrepareTextScript(char**tsname);
    int     Run(long curpc);
    int     RunScriptFunctionIfExists(char*tsname,int numParam, long iparam, long iparam2, long iparam3 = 0);
    int     RunTextScript(char*tsname);
    int     RunTextScriptIParam(char*tsname, long iparam);
    int     RunTextScript2IParam(char*tsname,long iparam,long param2);
    
    void    GetCallStack(char *buffer, int maxLines);
    void    GetScriptName(char *curScrName);
    // get the address of an exported variable in the script
    char    *GetSymbolAddress(char *);
    void    DumpInstruction(const ScriptOperation &op);
    
    // changes all pointer variables (ie. strings) to have the relative address, to allow
    // the data segment to be saved to disk
    void    FlattenGlobalData();
    // restores the pointers after a save
    void    UnFlattenGlobalData();

protected:
    bool    _Create(ccScript * scri, ccInstance * joined);
    // free the memory associated with the instance
    void    Free();

    bool    ResolveScriptImports(ccScript * scri);
    bool    FixupGlobalData(ccScript * scri);
    bool    CreateRuntimeCodeFixups(ccScript * scri);
	bool    ReadOperation(ScriptOperation &op, long at_pc);

    // Runtime fixups
    void    FixupInstruction(long code_index, char fixup_type, ScriptInstruction &instruction);
    void    FixupArgument(long code_index, char fixup_type, RuntimeScriptValue &argument);

    // Stack processing
    // Push writes new value and increments stack ptr;
    // stack ptr now points to the __next empty__ entry
    void    PushValueToStack(const RuntimeScriptValue &rval);
    void    PushDataToStack(int num_bytes);
    // Pop decrements stack ptr, returns last stored value and invalidates! stack tail;
    // stack ptr now points to the __next empty__ entry
    RuntimeScriptValue PopValueFromStack();
    // helper function to pop & dump several values
    void    PopValuesFromStack(int num_entries);
    void    PopDataFromStack(int num_bytes);
    // Return stack ptr at given offset from stack head;
    // Offset is in data bytes; program stack ptr is __not__ changed
    RuntimeScriptValue GetStackPtrOffsetFw(int fw_offset);
    // Return stack ptr at given offset from stack tail;
    // Offset is in data bytes; program stack ptr is __not__ changed
    RuntimeScriptValue GetStackPtrOffsetRw(int rw_offset);
};

#endif // __CC_INSTANCE_H
