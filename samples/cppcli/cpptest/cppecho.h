#pragma once

#include "stdafx.h" // Ensure stdafx.h is included

// Import the new NetEFI namespaces
using namespace NetEFI::Runtime;
using namespace NetEFI::Functions;

namespace cpptest
{
    [Computable( "cppecho", "s", "Returns the input string." )]
    public ref class CppEcho : public CustomFunction<String^, String^>
    {
    public:
        virtual String^ Execute( String^ s, Context^ context ) override
        {
            return s;
        }
    };
}