#pragma once

#include "stdafx.h"

using namespace System::Numerics;
using namespace NetEFI::Runtime;
using namespace NetEFI::Functions;

namespace cpptest
{
    [Computable( "cppsum", "a, b", "Calculates the complex sum of two scalars." )]
    public ref class CppSum : public CustomFunction<Complex, Complex, Complex>
    {
    public:
        virtual Complex Execute( Complex a, Complex b, Context^ context ) override
        {
            return a + b;
        }
    };
}