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

// ����� �����.
BOOL WINAPI DllEntryPoint( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved ) {

    switch ( dwReason ) {

        // DLL ������������ �� �������� ������������ ��������
        case DLL_PROCESS_ATTACH: { 

            if ( !LoadAssemblies( hinstDLL ) ) { 
                
                break;
            }

            break;
        }

        // �������� �����
        case DLL_THREAD_ATTACH: { break; }

        // ����� ��������� �����������
        case DLL_THREAD_DETACH: { break; }

        // DLL ����������� �� ��������� ������������ ��������
        case DLL_PROCESS_DETACH: { break; }
    }

    return TRUE; // ������������ ������ ��� DLL_PROCESS_ATTACH
}

#pragma managed