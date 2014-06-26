#include "stdafx.h"
#include "netefi.h"

using namespace NetEFI;


bool LoadAssemblies() {

    if ( !Manager::Initialize() ) return false;

    return Manager::LoadAssemblies();
}


#pragma unmanaged

BOOL WINAPI DllEntryPoint( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved ) {

    switch ( dwReason ) {

        // DLL проецируется на адресное пространство процесса
        case DLL_PROCESS_ATTACH: { 

            try { LoadAssemblies(); } catch (...) {}

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