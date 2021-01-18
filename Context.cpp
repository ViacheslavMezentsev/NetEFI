#include "stdafx.h"
#include "Manager.h"
#include "Context.h"

using namespace NetEFI;

bool Context::IsUserInterrupted::get()
{
    return ::isUserInterrupted == NULL ? false : ::isUserInterrupted();
}


bool Context::IsDefined( String ^ name )
{
    return default[ name ] != nullptr;
}


IFunction ^ Context::default::get( String ^ name )
{
    IFunction ^ res = nullptr;

    for each ( AssemblyInfo ^ assembly in Manager::Assemblies )
    {
        for each ( IFunction ^ func in assembly->Functions )
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
