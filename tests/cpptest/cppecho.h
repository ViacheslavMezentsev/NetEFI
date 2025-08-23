#pragma once

#include "stdafx.h" // Ensure stdafx.h is included

// Import the new NetEFI namespaces
using namespace NetEFI::Computables;
using namespace NetEFI::Design;
using namespace NetEFI::Functions;

namespace cpptest
{
    [Computable( "cppecho", "s", "Returns the input string." )]
    public ref class CppEcho : public MathcadFunction<String^, String^>
    {
    public:
        virtual String^ Execute( String^ s, Context^ context ) override
        {
            return s;
        }
    };
}