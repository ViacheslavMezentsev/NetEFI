// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <vector>
#include <string>
#include <windows.h>
#include <msclr/marshal_cppstd.h>
#include "mcadincl.h"

#pragma region using namespaces

using namespace System;
using namespace System::Numerics;
using namespace System::IO;
using namespace System::Text;
using namespace System::Linq;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Diagnostics;
using namespace System::Globalization;
using namespace System::Collections::Generic;

using namespace NetEFI::Computables;
using namespace NetEFI::Design;
using namespace NetEFI::Runtime;

using namespace msclr::interop;

#pragma endregion
