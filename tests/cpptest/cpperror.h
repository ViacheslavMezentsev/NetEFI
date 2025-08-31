#pragma once

#include "stdafx.h"

// Ensure all required namespaces are included, ideally via stdafx.h
using namespace System;
using namespace System::Numerics;
using namespace NetEFI::Functions;
using namespace NetEFI::Runtime;

namespace CppTest
{
    // 1. Define the possible errors for this function using attributes.
    //    The syntax is slightly different from C#: [attribute_name(args)]
    [Error( "Error1", "cpperror: This is custom error message 1." )]
    [Error( "Error2", "cpperror: This is custom error message 2." )]
    [Error( "Error3", "cpperror: This is custom error message 3." )]
    [Error( "Error4", "cpperror: This is custom error message 4." )]

    // 2. The Computable attribute remains the same.
    [Computable( "cpperror", "n", "Throws a custom error specified by its index n." )]
    public ref class CppError : public CustomFunction<Complex, String^>
    {
    public:
        // 3. The static 'Errors' array is NO LONGER NEEDED and should be deleted.
        // static array<String^>^ Errors = ...

        // 4. Implement the Execute method.
        virtual String^ Execute( Complex n, Context^ context ) override
        {
            int errorIndex = ( int ) n.Real;

            String^ errorKey;

            switch ( errorIndex )
            {
                case 1:
                    errorKey = "Error1";
                    break;
                case 2:
                    errorKey = "Error2";
                    break;
                case 3:
                    errorKey = "Error3";
                    break;
                case 4:
                    errorKey = "Error4";
                    break;
                default:
                    // Throw a standard exception if the user provides an invalid index.
                    // The host will catch this and log it as a critical error.
                    throw gcnew ArgumentOutOfRangeException(
                        "n",
                        String::Format( "The provided error index '{0}' is not valid. It must be between 1 and 4.", errorIndex )
                    );
            }

            // Throws the custom EFIException using the resolved key.
            // The second parameter is the 1-based index of the argument causing the error.
            throw gcnew EFIException( errorKey, 1 );
        }
    };
}