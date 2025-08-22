#include "stdafx.h"
#include "Manager.h"
#include "ContextImpl.h"

using namespace NetEFI;

extern CMathcadEfi MathcadEfi;

bool ContextImpl::IsUserInterrupted::get()
{
    return ( MathcadEfi.isUserInterrupted != nullptr ) && MathcadEfi.isUserInterrupted();
}


bool ContextImpl::IsDefined( String ^ name )
{
    return default[ name ] != nullptr;
}


IComputable^ ContextImpl::default::get( String ^ name )
{
    IComputable^ res = nullptr;

    for each ( AssemblyInfo ^ assembly in Manager::Assemblies )
    {
        for each (IComputable ^ func in assembly->Functions )
        {
            if ( func->Info->Name->Equals( name ) )
            {
                res = func;
                break;
            }
            else
            {
                continue;
            }
        }

        if ( res != nullptr ) break;
    }

    return res;
}


void ContextImpl::LogInfo( String ^ text )
{ 
    Manager::LogInfo( text );
}


void ContextImpl::LogInfo( String ^ format, ... array<Object ^> ^ list )
{ 
    Manager::LogInfo( format, list );
}


void ContextImpl::LogError( String ^ text )
{ 
    Manager::LogError( text );
}


void ContextImpl::LogError( String ^ format, ... array<Object ^> ^ list )
{ 
    Manager::LogError( format, list );
}
