#pragma once

#include "stdafx.h"

namespace NetEFI
{
    public ref class AssemblyInfo
    {
    public:
        String ^ Path;
        List < IComputable ^ > ^ Functions;

        AssemblyInfo( String ^ path )
        {
            Path = path;
            Functions = gcnew List< IComputable^ >();
        }
    };
}
