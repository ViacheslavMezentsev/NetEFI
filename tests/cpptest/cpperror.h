#pragma once

using namespace System;
using namespace System::Numerics;
using namespace System::Text;

using namespace NetEFI;

public ref class cpperror: public IFunction
{
public:

    static array<String^>^ Errors = gcnew array<String^>(4)
    { 
        "cpperror: text example 1",
        "cpperror: text example 2", 
        "cpperror: text example 3",
        "cpperror: text example 4"
    };

    virtual property FunctionInfo^ Info
    {
        FunctionInfo^ get()
        { 
            return gcnew FunctionInfo( "cpperror", "s", "return string",
                String::typeid, gcnew array<Type^> { Complex::typeid } );
        }
    }

    virtual FunctionInfo^ GetFunctionInfo( String^ lang ) { return Info; }

    virtual bool NumericEvaluation( array< Object^ > ^ args, [Out] Object ^ % result, Context ^ % context )
    {
        throw gcnew EFIException( ( int ) ( ( Complex ) args[0] ).Real, 1 );
    }
};
