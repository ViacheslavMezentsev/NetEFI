#include "stdafx.h"
#include "netefi.h"
#include "Manager.h"


using namespace NetEFI;

bool LoadAssemblies()
{
    return Manager::Initialize() ? Manager::LoadAssemblies() : false;
}


#pragma unmanaged

// Common Language Runtime Loader and DllMain
// http://msdn.microsoft.com/en-us/library/aa290048(v=vs.71).aspx
BOOL WINAPI DllEntryPoint( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved ) 
{
    switch ( dwReason )
    {
        // DLL проецируется на адресное пространство процесса
        case DLL_PROCESS_ATTACH:
        { 
            try
            {
                LoadAssemblies();
            }
            catch (...) {}

            break;
        }

        // Создаётся поток.
        case DLL_THREAD_ATTACH: { break; }

        // Поток корректно завершается.
        case DLL_THREAD_DETACH: { break; }

        // DLL отключается от адресного пространства процесса.
        case DLL_PROCESS_DETACH: { break; }
    }

    // Используется только для DLL_PROCESS_ATTACH.
    return TRUE;
}

#pragma managed
