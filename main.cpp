#include "stdafx.h"
#include "netefi.h"

using namespace NetEFI;

bool LoadAssemblies( HINSTANCE );


bool LoadAssemblies( HINSTANCE hInstance ) {

    if ( !Manager::Initialize() ) return false;

    return Manager::LoadAssemblies( hInstance );
}


#pragma unmanaged

BOOL WINAPI DllEntryPoint( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved ) {

    switch ( dwReason ) {

        // DLL проецируется на адресное пространство процесса
        case DLL_PROCESS_ATTACH: { 

			// C++/CLI Wrapping Managed Code for Unmanaged Use
			// http://stackoverflow.com/questions/9944539/c-cli-wrapping-managed-code-for-unmanaged-use
			// Specific to C++/CLI, you can write a free function and apply 
			// the __declspec(dllexport) attribute to it. The compiler will 
			// generate a stub that exports the function so you can call it 
			// from your C++ code with LoadLibrary + GetProcAddress. 
			// The stub automatically loads the CLR. This is very easy to get 
			// going but is pretty inflexible since you are only exposing a simple 
			// function and not a class.
			LoadAssemblies( hinstDLL );

            break;
        }

        // создаётся поток
        case DLL_THREAD_ATTACH: { break; }

        // поток корректно завершается
        case DLL_THREAD_DETACH: { break; }

        // DLL отключается от адресного пространства процесса
        case DLL_PROCESS_DETACH: { break; }
    }

    return TRUE; // используется только для DLL_PROCESS_ATTACH
}

#pragma managed