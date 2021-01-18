// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include "mcadincl.h"

extern PCREATE_USER_FUNCTION CreateUserFunction;
extern PCREATE_USER_ERROR_MESSAGE_TABLE CreateUserErrorMessageTable;
extern PMATHCAD_ALLOCATE MathcadAllocate;
extern PMATHCAD_FREE MathcadFree;
extern PMATHCAD_ARRAY_ALLOCATE MathcadArrayAllocate;
extern PMATHCAD_ARRAY_FREE MathcadArrayFree;
extern PIS_USER_INTERRUPTED isUserInterrupted;
