#include "stdafx.h"
#include <msclr\marshal_cppstd.h>
#include "netefi.h"


using namespace std;
using namespace msclr::interop;
using namespace NetEFI;


bool LoadAssemblies( HINSTANCE hInstance ) {

    if ( !Manager::Initialize() ) return false;

    return Manager::LoadAssemblies( hInstance );
}


#pragma unmanaged

// Точка входа.
BOOL WINAPI DllEntryPoint( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved ) {

    switch ( dwReason ) {

        // DLL проецируется на адресное пространство процесса
        case DLL_PROCESS_ATTACH: { 

            if ( !LoadAssemblies( hinstDLL ) ) { 
                
                break;
            }

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