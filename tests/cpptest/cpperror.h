#pragma once

#include "stdafx.h"

using namespace System::Numerics;
using namespace NetEFI::Computables;
using namespace NetEFI::Design;
using namespace NetEFI::Functions;
using namespace NetEFI::Runtime;

namespace cpptest
{
    [Computable( "cpperror", "n", "Throws a custom error specified by index n." )]
    public ref class CppError : public MathcadFunction<Complex, String^>
    {
    public:
        // The list of custom error messages for this function.
        static array<String^>^ Errors = gcnew array<String^> {
            "cpperror: This is custom error message 1.",
                "cpperror: This is custom error message 2.",
                "cpperror: This is custom error message 3.",
                "cpperror: This is custom error message 4."
        };

        virtual String^ Execute( Complex n, Context^ context ) override
        {
            // Throws a custom EFIException.
            throw gcnew EFIException( ( int ) n.Real, 1 );
        }
    };
}