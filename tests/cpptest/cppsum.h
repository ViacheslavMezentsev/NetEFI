#pragma once

#include "stdafx.h"

using namespace System::Numerics;
using namespace NetEFI::Computables;
using namespace NetEFI::Design;
using namespace NetEFI::Functions;

namespace cpptest
{
    [Computable( "cppsum", "a, b", "Calculates the complex sum of two scalars." )]
    public ref class CppSum : public MathcadFunction<Complex, Complex, Complex>
    {
    public:
        virtual Complex Execute( Complex a, Complex b, Context^ context ) override
        {
            return a + b;
        }
    };
}